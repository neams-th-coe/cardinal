# syntax=docker/dockerfile:1
# Needed for RUN --mount=type=bind below.

# Defaults to "latest" since contrib/moose isn't fetched yet to know the
# exact tag it needs; see the version-match RUN step below.
ARG MOOSEDEV_TAG=latest

# Where the final, self-contained `make install` output goes (see the
# very end of this file). Overridable via --build-arg for a different
# deployment layout.
ARG CARDINAL_PREFIX=/opt/cardinal

# Cardinal's own ENABLE_* build switches, as ARGs rather than hardcoded
# twice: the "build" stage's `make`/`make build_X` steps and the
# "install" stage's `make install` (which re-examines the same targets
# against a bind-mounted /cardinal -- see below) both need the exact
# same values, or `make install` recomputes a dependency's cmake flags
# differently than what it was actually built with and triggers a real
# (if otherwise harmless) reconfigure attempt. A single source of truth
# here means adding or flipping one of these later can't silently
# desync the two stages.
ARG ENABLE_NEK=yes
ARG ENABLE_OPENMC=yes
ARG ENABLE_DAGMC=yes
ARG ENABLE_DOUBLE_DOWN=yes

# A plain git clone leaves public submodules uninitialized; fetch them
# here so users don't have to run scripts/get-dependencies.sh first.
# This stage's RUN always re-executes (its cache key is keyed off .git,
# which changes on every commit), but downstream COPY --from steps are
# content-addressed, so the per-dependency cache granularity below is
# unaffected by that.
FROM idaholab/moose-dev:${MOOSEDEV_TAG} AS submodules-moose
ARG MOOSEDEV_TAG
ARG CARDINAL_PREFIX
WORKDIR /cardinal
COPY .git .git
COPY .gitmodules .gitmodules

# Fetched and checked on its own, before the other submodules branch off
# of this stage below, so a version mismatch fails fast without spending
# time/bandwidth on them first.
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

# Configure MOOSE once, here, before anything is ever compiled. This
# generates the real, final conf_vars.mk (setting PREFIX, so
# CONTRIB_INSTALL_DIR's own ?= default would otherwise disagree with
# it) and MooseConfig.h up front, instead of deferring MooseConfig.h to
# a placeholder swapped for the real thing right before the final
# build. Doing this after anything is already compiled regenerates
# MooseConfig.h with a fresh timestamp -- newer than every object file
# that already #includes it -- which Make sees as needing a full
# rebuild (confirmed empirically: a `make install` retrofitted onto an
# already-built tree triggered one). Doing it first avoids that
# entirely, since nothing yet exists for the new timestamp to
# invalidate.
RUN . /environment && cd contrib/moose && ./configure --prefix=${CARDINAL_PREFIX}

# Each remaining submodule gets its own stage, branching off
# submodules-moose (so none of them start until the version check above
# has passed) rather than one combined RUN -- lets BuildKit fetch them
# concurrently instead of one at a time, and each downstream COPY --from
# below only misses cache if that specific submodule's pin changed.
FROM submodules-moose AS submodules-nekrs
RUN git submodule update --init --progress contrib/nekRS

FROM submodules-moose AS submodules-openmc
RUN git submodule update --init --progress contrib/openmc

FROM submodules-moose AS submodules-nuclear-data
RUN git submodule update --init --progress contrib/nuclear_data

FROM submodules-moose AS submodules-dagmc
RUN git submodule update --init --progress contrib/DAGMC

FROM submodules-moose AS submodules-moab
RUN git submodule update --init --progress contrib/moab

FROM submodules-moose AS submodules-embree
RUN git submodule update --init --progress contrib/embree

FROM submodules-moose AS submodules-double-down
RUN git submodule update --init --progress contrib/double-down

FROM idaholab/moose-dev:${MOOSEDEV_TAG} AS build
ARG CARDINAL_PREFIX
ARG ENABLE_NEK
ARG ENABLE_OPENMC
ARG ENABLE_DAGMC
ARG ENABLE_DOUBLE_DOWN

ENV HDF5_ROOT=/opt/petsc
ENV BASH_ENV=/environment

SHELL ["/bin/bash", "-c"]

# Routes OpenMC/NekRS/MOAB/DAGMC/Embree/Double-Down's own `make install`
# steps straight into the final prefix instead of Cardinal's default
# ($(CARDINAL_DIR)/install), so `make install` at the very end doesn't
# need to relocate them afterward -- confirmed empirically that without
# this, the installed binary still depends on the original build tree.
ENV CONTRIB_INSTALL_DIR=${CARDINAL_PREFIX}
ENV NEKRS_HOME=${CARDINAL_PREFIX}
ENV ENABLE_NEK=${ENABLE_NEK}
ENV ENABLE_OPENMC=${ENABLE_OPENMC}
ENV ENABLE_DAGMC=${ENABLE_DAGMC}
ENV ENABLE_DOUBLE_DOWN=${ENABLE_DOUBLE_DOWN}

WORKDIR /cardinal

# Tiny (~49KB) and rarely changes.
COPY --from=submodules-nuclear-data /cardinal/contrib/nuclear_data contrib/nuclear_data

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
COPY --from=submodules-moose /cardinal/contrib/moose/framework/build.mk contrib/moose/framework/build.mk
COPY --from=submodules-moose /cardinal/contrib/moose/framework/moose.mk contrib/moose/framework/moose.mk
COPY --from=submodules-moose /cardinal/contrib/moose/framework/app.mk contrib/moose/framework/app.mk
COPY --from=submodules-moose /cardinal/contrib/moose/framework/contrib/neml2.mk contrib/moose/framework/contrib/neml2.mk
COPY --from=submodules-moose /cardinal/contrib/moose/modules/modules.mk contrib/moose/modules/modules.mk
# Real, final versions (./configure already ran in submodules-moose) --
# not placeholders -- so PREFIX/CONTRIB_INSTALL_DIR and MooseConfig.h
# are consistent from the very first dependency build below.
COPY --from=submodules-moose /cardinal/contrib/moose/conf_vars.mk contrib/moose/conf_vars.mk
COPY --from=submodules-moose /cardinal/contrib/moose/framework/include/base/MooseConfig.h contrib/moose/framework/include/base/MooseConfig.h

# Ordered by dependency first (embree and moab are independent leaves;
# double_down needs both; dagmc needs double_down), then by frequency of
# change among independencies (embree: 1 commit touching it, moab: 9,
# double-down: 3, dagmc: 7, nekRS: 33, openmc: 82) so each layer
# invalidates as few of the later ones as possible.
#
# config/*.mk and contrib/{nekRS,openmc,DAGMC,moab,double-down} are only
# existence-checked here (same ls-based check config/check_deps.mk uses
# for all of them, run unconditionally once ENABLE_DAGMC/
# ENABLE_DOUBLE_DOWN=yes regardless of which target is being built) --
# not needed for real until their own layers below. The HIT parser
# placeholder satisfies the same kind of compile-time check for
# Cardinal's own CFLAGS/LDFLAGS (which leak "-include .../hit...") into
# third-party sub-builds like HYPRE's; removed before the final build.
RUN mkdir -p contrib/nekRS contrib/openmc contrib/DAGMC contrib/moab contrib/double-down \
             contrib/moose/framework/contrib/hit && \
    touch contrib/nekRS/DOCKERFILE_PLACEHOLDER \
          contrib/openmc/DOCKERFILE_PLACEHOLDER \
          contrib/DAGMC/DOCKERFILE_PLACEHOLDER \
          contrib/moab/DOCKERFILE_PLACEHOLDER \
          contrib/double-down/DOCKERFILE_PLACEHOLDER \
          contrib/moose/framework/contrib/hit/DOCKERFILE_PLACEHOLDER \
          config/nekrs.mk config/openmc.mk

COPY --from=submodules-embree /cardinal/contrib/embree contrib/embree
RUN make build_embree -j$(nproc)

RUN rm -f contrib/moab/DOCKERFILE_PLACEHOLDER
COPY --from=submodules-moab /cardinal/contrib/moab contrib/moab
RUN make build_moab -j$(nproc)

RUN rm -f contrib/double-down/DOCKERFILE_PLACEHOLDER
COPY --from=submodules-double-down /cardinal/contrib/double-down contrib/double-down
RUN make build_doubledown -j$(nproc)

RUN rm -f contrib/DAGMC/DOCKERFILE_PLACEHOLDER
COPY --from=submodules-dagmc /cardinal/contrib/DAGMC contrib/DAGMC
RUN make build_dagmc -j$(nproc)

RUN rm -f contrib/nekRS/DOCKERFILE_PLACEHOLDER
COPY config/nekrs.mk config/nekrs.mk
COPY --from=submodules-nekrs /cardinal/contrib/nekRS contrib/nekRS
RUN make build_nekrs -j$(nproc) || \
      (cat build/nekrs/HYPRE_BUILD-prefix/src/HYPRE_BUILD/src/config.log; exit 1)

RUN rm -f contrib/openmc/DOCKERFILE_PLACEHOLDER
COPY config/openmc.mk config/openmc.mk
COPY --from=submodules-openmc /cardinal/contrib/openmc contrib/openmc
RUN make build_openmc -j$(nproc)

RUN rm -f contrib/moose/framework/contrib/hit/DOCKERFILE_PLACEHOLDER

# The rest of MOOSE (framework/src, modules/*, scripts, python, ...)
# wasn't needed until now -- the dependency builds above only needed
# framework/build.mk (copied separately, above). Copying it here, before
# Cardinal's own COPY . ., keeps this layer cache-keyed on the
# submodules stage rather than on Cardinal's working tree, so a commit
# that doesn't bump the moose submodule (the common case) doesn't force
# it to be re-copied.
COPY --from=submodules-moose /cardinal/contrib/moose contrib/moose
# COPY merges rather than replaces, so this fills in Cardinal's own
# tracked files without disturbing contrib/moose above. --exclude keeps
# .git/Dockerfile/.dockerignore (none of which are app source, but
# .git's content differs every commit on its own) out of this layer's
# cache key, which would otherwise force the whole compile below to
# redo on every commit. A .dockerignore entry can't do this instead --
# it would also hide .git from submodules-moose's own COPY .git .git,
# which genuinely needs it.
COPY --exclude=.git --exclude=Dockerfile --exclude=.dockerignore . .

RUN make -j$(nproc)

# Default build target (the last stage, and the CI workflow passes no
# --target): just moose-dev with Cardinal's self-contained install --
# adds ~512M rather than the ~3.2G the "build" stage above carries
# (full submodule source, CMake build directories, .git). `docker build
# --target build .` gets the full dev tree instead, e.g. to live-patch
# and rebuild a single already-built dependency in place.
FROM idaholab/moose-dev:${MOOSEDEV_TAG} AS install
ARG CARDINAL_PREFIX
ARG ENABLE_NEK
ARG ENABLE_OPENMC
ARG ENABLE_DAGMC
ARG ENABLE_DOUBLE_DOWN

ENV BASH_ENV=/environment
SHELL ["/bin/bash", "-c"]

# NEKRS_HOME is the only one of these actually read at runtime (by
# NekRS itself, not just by Cardinal's own Makefile) -- everything the
# RUN below passes inline is a pure Makefile/CMake build-time flag with
# no runtime reads anywhere in the source, confirmed by grep.
ENV NEKRS_HOME=${CARDINAL_PREFIX}
ENV PATH=${CARDINAL_PREFIX}/bin:$PATH

# The standard MOOSE/Cardinal install mechanism (see
# https://mooseframework.inl.gov/application_development/build_system.html#make-install):
# copies cardinal-opt and every required library into CONTRIB_INSTALL_DIR
# (routed to CARDINAL_PREFIX, matching the "build" stage, so NekRS/
# OpenMC/MOAB/DAGMC/Embree/Double-Down land in the same tree rather than
# needing their RPATHs patched after the fact -- confirmed empirically
# that without that routing, the installed binary still depends on the
# original /cardinal build tree existing). Also installs MOOSE's/the
# enabled modules' data directories and Cardinal's own INSTALLABLE_DIRS
# test/tutorial inputs. MOOSE_SKIP_DOCS avoids a hard dependency on the
# large_media submodule, which this image doesn't fetch.
#
# /cardinal is bind-mounted, not copied, so "build" doesn't carry a
# redundant copy of CARDINAL_PREFIX alongside the full dev tree. Needs
# ,rw: build_embree/build_moab/etc. are .PHONY, so make install's
# dependency chain always re-runs their recipe (`$(MAKE) -C $(BUILDDIR)
# install`), and CMake's own generated install target unconditionally
# writes install_manifest.txt into its build directory on every
# invocation, not just when something actually changed. Harmless here:
# BuildKit bind-mount writes during a RUN go to a throwaway overlay,
# never back into "build"'s actual layers. Every ENABLE_* flag below
# also has to match the "build" stage exactly (passed from the same
# ARGs, not retyped here) -- otherwise make install recomputes a
# dependency's cmake flags differently than what it was actually built
# with and triggers a real reconfigure (confirmed separately, before
# ,rw was added here: that reconfigure attempt failed outright against
# a read-only mount).
#
# .git is mounted separately, from submodules-moose rather than build:
# "build" deliberately excludes .git from its own COPY . . (see above),
# but INSTALLABLE_DIRS's install_copy_inputs.sh hard-requires a real
# `git rev-parse --show-toplevel` to resolve (unlike MooseRevision.h
# generation, which tolerates a missing .git) -- confirmed by the
# install failing with "fatal: not a git repository" without this.
RUN --mount=type=bind,from=build,source=/cardinal,target=/cardinal,rw \
    --mount=type=bind,from=submodules-moose,source=/cardinal/.git,target=/cardinal/.git \
    cd /cardinal && \
    CONTRIB_INSTALL_DIR=${CARDINAL_PREFIX} \
    ENABLE_NEK=${ENABLE_NEK} \
    ENABLE_OPENMC=${ENABLE_OPENMC} \
    ENABLE_DAGMC=${ENABLE_DAGMC} \
    ENABLE_DOUBLE_DOWN=${ENABLE_DOUBLE_DOWN} \
    MOOSE_SKIP_DOCS=1 \
    make install -j$(nproc)
