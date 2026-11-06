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
    git submodule update --init --progress contrib/nuclear_data && \
    git submodule update --init --progress contrib/DAGMC && \
    git submodule update --init --progress contrib/moab && \
    git submodule update --init --progress contrib/embree && \
    git submodule update --init --progress contrib/double-down

FROM idaholab/moose-dev:${MOOSEDEV_TAG}

ENV HDF5_ROOT=/opt/petsc
ENV BASH_ENV=/environment

SHELL ["/bin/bash", "-c"]

ENV NEKRS_HOME=/cardinal/install
ENV ENABLE_DAGMC=yes
ENV ENABLE_DOUBLE_DOWN=yes

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
COPY config/moab.mk config/moab.mk
COPY config/dagmc.mk config/dagmc.mk
COPY config/embree.mk config/embree.mk
COPY config/double_down.mk config/double_down.mk
COPY --from=submodules /cardinal/contrib/moose/framework/build.mk contrib/moose/framework/build.mk
COPY --from=submodules /cardinal/contrib/moose/framework/moose.mk contrib/moose/framework/moose.mk
COPY --from=submodules /cardinal/contrib/moose/framework/app.mk contrib/moose/framework/app.mk
COPY --from=submodules /cardinal/contrib/moose/framework/contrib/neml2.mk contrib/moose/framework/contrib/neml2.mk
COPY --from=submodules /cardinal/contrib/moose/modules/modules.mk contrib/moose/modules/modules.mk

# Ordered by dependency first (embree and moab are independent leaves;
# double_down needs both; dagmc needs double_down), then by frequency of
# change among independencies (embree: 1 commit touching it, moab: 9,
# double-down: 3, dagmc: 7, nekRS: 33, openmc: 82) so each layer
# invalidates as few of the later ones as possible.
#
# config/*.mk, contrib/{nekRS,openmc,DAGMC,moab,double-down}, and the HIT
# parser are only existence-checked here (same ls-based check
# config/check_deps.mk uses for all of them, run unconditionally once
# ENABLE_DAGMC/ENABLE_DOUBLE_DOWN=yes regardless of which target is being
# built) -- not needed for real until their own layers below. Cardinal's
# own CFLAGS/LDFLAGS (incl. "-include .../MooseConfig.h") leak into
# third-party sub-builds like HYPRE's; MooseConfig.h is normally
# generated as a side effect of building $(app_EXEC), which none of
# these layers ask for. The placeholder satisfies the compile check;
# removed before the final build so it regenerates for real.
RUN mkdir -p contrib/nekRS contrib/openmc contrib/DAGMC contrib/moab contrib/double-down \
             contrib/moose/framework/contrib/hit \
             contrib/moose/framework/include/base && \
    touch contrib/nekRS/DOCKERFILE_PLACEHOLDER \
          contrib/openmc/DOCKERFILE_PLACEHOLDER \
          contrib/DAGMC/DOCKERFILE_PLACEHOLDER \
          contrib/moab/DOCKERFILE_PLACEHOLDER \
          contrib/double-down/DOCKERFILE_PLACEHOLDER \
          contrib/moose/framework/contrib/hit/DOCKERFILE_PLACEHOLDER \
          contrib/moose/framework/include/base/MooseConfig.h \
          config/nekrs.mk config/openmc.mk

COPY --from=submodules /cardinal/contrib/embree contrib/embree
RUN make build_embree -j$(nproc)

RUN rm -f contrib/moab/DOCKERFILE_PLACEHOLDER
COPY --from=submodules /cardinal/contrib/moab contrib/moab
RUN make build_moab -j$(nproc)

RUN rm -f contrib/double-down/DOCKERFILE_PLACEHOLDER
COPY --from=submodules /cardinal/contrib/double-down contrib/double-down
RUN make build_doubledown -j$(nproc)

RUN rm -f contrib/DAGMC/DOCKERFILE_PLACEHOLDER
COPY --from=submodules /cardinal/contrib/DAGMC contrib/DAGMC
RUN make build_dagmc -j$(nproc)

RUN rm -f contrib/nekRS/DOCKERFILE_PLACEHOLDER
COPY config/nekrs.mk config/nekrs.mk
COPY --from=submodules /cardinal/contrib/nekRS contrib/nekRS
RUN make build_nekrs -j$(nproc) || \
      (cat build/nekrs/HYPRE_BUILD-prefix/src/HYPRE_BUILD/src/config.log; exit 1)

RUN rm -f contrib/openmc/DOCKERFILE_PLACEHOLDER
COPY config/openmc.mk config/openmc.mk
COPY --from=submodules /cardinal/contrib/openmc contrib/openmc
RUN make build_openmc -j$(nproc)

# Remove the MooseConfig.h placeholder so the real build regenerates it
# -- otherwise its build-time mtime (newer than anything COPY'd from
# the checkout) could fool Make into thinking it's already up to date.
RUN rm -f contrib/moose/framework/include/base/MooseConfig.h \
           contrib/moose/framework/contrib/hit/DOCKERFILE_PLACEHOLDER

# The rest of MOOSE (framework/src, modules/*, scripts, python, ...)
# wasn't needed until now -- the dependency builds above only needed
# framework/build.mk (copied separately, above). Copying it here, before
# Cardinal's own COPY . ., keeps this layer cache-keyed on the
# submodules stage rather than on Cardinal's working tree, so a commit
# that doesn't bump the moose submodule (the common case) doesn't force
# it to be re-copied.
COPY --from=submodules /cardinal/contrib/moose contrib/moose
# COPY merges rather than replaces, so this fills in Cardinal's own
# tracked files without disturbing contrib/moose above.
COPY . .

RUN make -j$(nproc)
