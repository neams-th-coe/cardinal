# Defaults to "latest" since contrib/moose isn't fetched yet to know the
# exact tag it needs; see the version-match RUN step below.
ARG MOOSEDEV_TAG=latest

# A plain git clone leaves public submodules uninitialized; fetch them
# here so users don't have to run scripts/get-dependencies.sh first.
# This stage's RUN always re-executes (its cache key is keyed off .git,
# which changes on every commit), but downstream COPY --from steps are
# content-addressed, so the per-dependency cache granularity below is
# unaffected by that.
FROM idaholab/moose-dev:${MOOSEDEV_TAG} AS submodules
ARG MOOSEDEV_TAG
WORKDIR /cardinal
COPY .git .git
COPY .gitmodules .gitmodules

# Fetched and checked on its own, before the other (larger) submodules,
# so a version mismatch fails fast without spending time/bandwidth on
# them first.
RUN git submodule update --init --progress contrib/moose

# MOOSE's own version check (scripts/premake.py) only fires under a login
# shell or conda, neither of which a plain RUN gets, and even then only
# warns. Enforce a real match ourselves: ask scripts/versioner.py (the
# source premake.py itself uses) what moose-dev tag this commit requires,
# and compare it to MOOSE_APPTAINER_GENERATOR_TAG -- the image's own
# actual version, from /environment -- rather than MOOSEDEV_TAG, which is
# just what we asked for and is "latest" by default.
RUN . /environment && \
    cd contrib/moose && \
    required=$(python3 scripts/versioner.py moose-dev HEAD --json | \
      python3 -c "import json, sys; print(json.load(sys.stdin)['apptainer']['tag'])") && \
    actual="${MOOSE_APPTAINER_GENERATOR_TAG:-unknown}" && \
    if [ "$actual" != "$required" ]; then \
      echo "This is moose-dev:$actual (requested as moose-dev:$MOOSEDEV_TAG), but contrib/moose (HEAD) was tested against moose-dev:$required." >&2; \
      echo "Rebuild with: docker build --build-arg MOOSEDEV_TAG=$required ." >&2; \
      exit 1; \
    fi

RUN git submodule update --init --progress contrib/nekRS && \
    git submodule update --init --progress contrib/openmc && \
    git submodule update --init --progress contrib/nuclear_data

FROM idaholab/moose-dev:${MOOSEDEV_TAG}

ENV HDF5_ROOT=/opt/petsc
ENV BASH_ENV=/environment

SHELL ["/bin/bash", "-c"]

ENV NEKRS_HOME=/cardinal/install

WORKDIR /cardinal

# Tiny (~49KB) and rarely changes.
COPY --from=submodules /cardinal/contrib/nuclear_data contrib/nuclear_data

# Only the config/*.mk and MOOSE files actually needed to parse the
# Makefile, not their much larger, more frequently changing trees.
COPY Makefile ./
COPY config/check_deps.mk config/check_deps.mk
COPY config/check_modules.mk config/check_modules.mk
COPY config/check_nekrs.mk config/check_nekrs.mk
COPY config/add_flags.mk config/add_flags.mk
COPY --from=submodules /cardinal/contrib/moose/framework/build.mk contrib/moose/framework/build.mk
COPY --from=submodules /cardinal/contrib/moose/framework/moose.mk contrib/moose/framework/moose.mk
COPY --from=submodules /cardinal/contrib/moose/framework/app.mk contrib/moose/framework/app.mk
COPY --from=submodules /cardinal/contrib/moose/framework/contrib/neml2.mk contrib/moose/framework/contrib/neml2.mk
COPY --from=submodules /cardinal/contrib/moose/modules/modules.mk contrib/moose/modules/modules.mk

# NekRS changes less often than OpenMC, so build it first. openmc.mk,
# contrib/openmc, and the HIT parser are only existence-checked here,
# not needed for real until later.
COPY config/nekrs.mk config/nekrs.mk
COPY --from=submodules /cardinal/contrib/nekRS contrib/nekRS
# Cardinal's own CFLAGS/LDFLAGS (incl. "-include .../MooseConfig.h")
# leak into third-party sub-builds like HYPRE's; MooseConfig.h is
# normally generated as a side effect of building $(app_EXEC), which
# this layer never asks for. A placeholder satisfies the compile check;
# removed before the final build so it regenerates for real.
RUN mkdir -p contrib/openmc contrib/moose/framework/contrib/hit \
             contrib/moose/framework/include/base && \
    touch contrib/openmc/DOCKERFILE_PLACEHOLDER \
          contrib/moose/framework/contrib/hit/DOCKERFILE_PLACEHOLDER \
          contrib/moose/framework/include/base/MooseConfig.h \
          config/openmc.mk && \
    make build_nekrs || \
      (cat build/nekrs/HYPRE_BUILD-prefix/src/HYPRE_BUILD/src/config.log; exit 1)

RUN rm -f contrib/openmc/DOCKERFILE_PLACEHOLDER
COPY config/openmc.mk config/openmc.mk
COPY --from=submodules /cardinal/contrib/openmc contrib/openmc
RUN make build_openmc

# Remove the MooseConfig.h placeholder so the real build regenerates it
# -- otherwise its build-time mtime (newer than anything COPY'd from
# the checkout) could fool Make into thinking it's already up to date.
RUN rm -f contrib/moose/framework/include/base/MooseConfig.h \
           contrib/moose/framework/contrib/hit/DOCKERFILE_PLACEHOLDER

# COPY preserves checkout-time mtimes, which across separate CI runs
# are always newer than yesterday's cached build output -- Make would
# otherwise reconfigure and rebuild from scratch here. Backdating the
# source tree (rather than touching build/install to "now") matters:
# ExternalProject_Add targets like HYPRE rely on their own stamp files
# being internally consistent with each other, and bumping everything
# under build/ to ~the same timestamp in filesystem-traversal order
# scrambles that ordering, forcing a full rebuild anyway.
COPY . .
# The rest of MOOSE (framework/src, modules/*, scripts, python, ...)
# wasn't needed until now; COPY merges rather than replaces, so this
# fills it in without disturbing the subset already copied above.
COPY --from=submodules /cardinal/contrib/moose contrib/moose
RUN find . \( -path ./build -o -path ./install \) -prune -o \
    -exec touch -t 202001010000 {} +

RUN make -j$(nproc)
