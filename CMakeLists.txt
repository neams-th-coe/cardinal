cmake_minimum_required(VERSION 3.21)  # find_program(... NO_CACHE) used below
project(CardinalSuperbuild NONE)

# This is a *superbuild*: it never compiles Cardinal itself. It resolves and
# builds the third-party dependencies that have their own CMake sub-build
# (currently NekRS and OpenMC) and then hands off to Cardinal's existing,
# unmodified Makefile to compile the app. See cmake/PLAN.md for the design.

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR
    "Cardinal's CMake build must be configured out-of-source, e.g.\n"
    "  cmake -S ${CMAKE_SOURCE_DIR} -B <build-dir>")
endif()

include(ExternalProject)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(CardinalDependency)
include(CardinalMoosePrereq)

# The final "hand off to make" step (below) relies on the $(MAKE) recursive-
# invocation convention so it shares the *same* jobserver `cmake --build -jN`
# (or plain `make -jN` in this build dir) already establishes, rather than
# this CMakeLists.txt guessing a parallelism number of its own. That
# convention is specific to the Makefiles generator.
if(NOT CMAKE_GENERATOR MATCHES "Makefiles")
  message(FATAL_ERROR
    "Cardinal's CMake superbuild requires a Makefiles generator (got "
    "'${CMAKE_GENERATOR}'), so its final build step can share the "
    "invoking make's -j setting via the $(MAKE) convention.")
endif()

# -----------------------------------------------------------------------------
# Options mirroring the Makefile's own ENABLE_* switches (Makefile:41-51)
# -----------------------------------------------------------------------------
option(ENABLE_NEK         "Build Cardinal with NekRS coupling"    ON)
option(ENABLE_OPENMC      "Build Cardinal with OpenMC coupling"   ON)
option(ENABLE_DAGMC       "Build OpenMC with DAGMC support"       OFF)
option(ENABLE_DOUBLE_DOWN "Use Double-Down for DAGMC ray tracing" OFF)

# NekRS's own OCCA device backends -- off by default (matching what the
# container build effectively gets, since it has no GPU toolkits), but
# exposed here (rather than hardcoded) so `ccmake` can turn them back on
# for a GPU-enabled build. Named to match what they get forwarded as
# (both NekRS's own ENABLE_CUDA/HIP/OPENCL and OCCA's OCCA_ENABLE_*,
# below) rather than Cardinal's Makefile ENABLE_* switches above, which
# are a separate namespace (this superbuild's own top-level CMakeCache,
# not nekrs's nested one).
option(ENABLE_CUDA   "Build NekRS/OCCA with CUDA support"       OFF)
option(ENABLE_HIP    "Build NekRS/OCCA with HIP support"        OFF)
option(ENABLE_OPENCL "Build NekRS/OCCA with OpenCL support"     OFF)
option(ENABLE_DPCPP  "Build NekRS/OCCA with SYCL/DPCPP support" OFF)
option(ENABLE_METAL  "Build NekRS/OCCA with Metal support"      OFF)

# PETSc's configure_petsc.sh always builds its downloaded OpenBLAS with
# DYNAMIC_ARCH=1 -- kernels for every x86 microarchitecture it knows
# about (Prescott, Core2, ..., Zen, SkylakeX, ...), auto-selected at
# runtime via CPUID. Safe on a cluster where the build host and the
# nodes you'll actually run on might differ, but much slower to build.
# ON (safe) by default; turn OFF with ccmake only when you know the
# build host's CPU is representative of every node you'll run on.
option(OPENBLAS_DYNAMIC_ARCH
  "Build PETSc's OpenBLAS for many x86 microarchitectures (portable across mixed-CPU clusters, slower); turn OFF to build only for the host's own CPU" ON)

if(ENABLE_DOUBLE_DOWN AND NOT ENABLE_DAGMC)
  message(FATAL_ERROR "ENABLE_DOUBLE_DOWN requires ENABLE_DAGMC")
endif()

# DAGMC/MOAB/Embree/double-down sub-builds are not wired up yet (Phase 2).
if(ENABLE_DAGMC)
  message(FATAL_ERROR "ENABLE_DAGMC is not implemented yet in the CMake build (Phase 2); configure with -DENABLE_DAGMC=OFF for now.")
endif()

# Reuse CMake's own standard CMAKE_BUILD_TYPE rather than inventing a
# separate variable -- this superbuild's own project() has no compiled
# languages (it only forwards this to the dependency sub-builds below),
# but CMAKE_BUILD_TYPE is still the conventional place for it.
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type forwarded to dependency CMake sub-builds (Release or Debug)")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Release Debug)

# -----------------------------------------------------------------------------
# Layout
#
# Everything -- mirrored app source, contrib/<dep>, build/<dep>, install/ --
# lives directly in the CMake build tree, so the result of a CMake build
# looks like the result of an in-tree Makefile build (cardinal-opt, lib/,
# src/, ... right at the top), not nested inside an extra subdirectory. The
# one real collision this creates is Cardinal's own hand-written top-level
# Makefile against CMake's own generated one of the same name; mirror_source
# (below) resolves that by mirroring everything else normally but copying
# Makefile itself under a different name (Makefile.cardinal), and the final
# "cardinal" target invokes that explicitly via `$(MAKE) -f Makefile.cardinal`.
# See cmake/PLAN.md.
# -----------------------------------------------------------------------------
set(CARDINAL_STAGE_DIR   ${CMAKE_BINARY_DIR})
set(CARDINAL_INSTALL_DIR ${CARDINAL_STAGE_DIR}/install)

# Mirror everything except the large, separately-managed submodules and
# CMake/Make build byproducts. Reruns every build (add_custom_target has no
# tracked OUTPUT, so it's always considered out of date) -- the same role as
# the manual `rsync -a` in SYNC_tmp/SYNC_nvme, but automatic.
#
# -rlpgo, not -a: -a is -rlptgoD, and the -t (preserve source mtime) is
# exactly wrong here. The source tree can keep being edited while a long
# build runs, and Cardinal's Makefile relies on ordinary mtime comparisons
# to decide what needs recompiling -- copying a source mtime through
# verbatim (rather than stamping the destination with the actual copy
# time) is what copy_directory_if_newer avoids too. -D (devices/specials)
# is dropped as well since it's meaningless for a source tree.
#
# --update is required alongside dropping -t, not optional: without -t,
# the destination's mtime is just "whenever it was last copied", which
# will essentially never equal the source's current mtime -- so rsync's
# default quick check (size+mtime) sees every file as changed and
# re-copies (and re-stamps) the entire tree on every single build,
# forcing a full recompile+relink every time regardless of whether
# anything actually changed. --update skips a file whenever the
# destination is already newer than the source (only comparing
# timestamps, not hashing every file's content) -- a genuinely-
# unchanged file keeps its stable destination mtime, and a real source
# edit (whose mtime is necessarily newer than the last mirror copy)
# gets re-copied with a fresh one. Matches copy_directory_if_newer's
# own actual semantics more closely than a content-hash comparison
# would, without trusting a source mtime a concurrent edit could make
# misleading.
# Cardinal builds in-place: object files, libraries, dependency files,
# and the *Revision.h headers all land inline next to their sources
# (e.g. src/base/CardinalApp.o), not confined to /build. None of that
# exists in the pristine source tree, so --delete removed it all on
# every single mirror_source run, as soon as it was produced -- forcing
# a full rebuild+relink every time regardless of the mtime/--update fix
# above (confirmed live: CardinalRevision.h was getting deleted and
# regenerated on every build; a missing prerequisite is obviously
# "changed", no mtime trickery needed for that). Rather than try to
# comprehensively protect every in-tree generated pattern Cardinal's
# Makefile happens to produce, just drop --delete: the cost is a stale
# copy lingering if a source file is later removed/renamed upstream,
# which is far cheaper than silently rebuilding the world every time.
# Just copy everything (only excluding what we must for functional
# reasons below), rather than trying to filter out generated/ignored
# content: a .gitignore-pattern filter got this wrong in a way that
# actually broke tests -- .gitignore only hides *untracked* files from
# git, so a file that's explicitly tracked despite matching a broad
# ignore pattern (e.g. test/tests/.../model.xml, tracked despite the
# repo's blanket "*.xml" ignore -- that pattern exists to keep
# *generated* XML out, not this fixture) stays tracked and visible to
# git regardless, but rsync's --filter has no such "tracked overrides
# ignore" concept and excluded it unconditionally. Mirroring exactly
# what `git ls-files` tracks instead would dodge that specific problem,
# but silently misses any file the user has created but not yet even
# `git add`ed -- also wrong. Simplest and most robust: don't try to be
# clever about what belongs in the mirror at all.
#
# --exclude=/contrib covers all 14 submodules under it (the 4 this
# superbuild manages -- moose/nekRS/openmc/nuclear_data, each resolved
# independently by its own cardinal_add_submodule_dependency call above
# or below -- plus 10 more for not-yet-implemented Phase 2 features
# like DAGMC/SAM/bison/embree/... that we don't want mirrored at all
# even if a user happens to have them checked out locally). Confirmed
# nothing else lives directly under contrib/ -- every path there is a
# submodule mount point, nothing Cardinal-specific.
#
# --exclude=/Makefile, plus the separate copy_if_different below: since
# CARDINAL_STAGE_DIR is now the CMake build directory itself (see
# "Layout" above), rsync-ing Makefile normally would collide with
# CMake's own generated one of the same name. copy_if_different gives
# the renamed copy the same "only touch it if content actually
# changed" correctness the --update rsync above has, without needing
# rsync just for one file.
# test/tests/nek_ci is its own submodule outside contrib/, resolved
# independently (like contrib's own submodules) only when ENABLE_NEK is
# on -- see that block below. Exclude it here too in that case so the
# two don't fight over the same destination; when ENABLE_NEK is off
# there's no dedicated resolver for it, so let it just get copied
# normally along with everything else (a no-op if it isn't checked out).
set(_mirror_excludes --exclude=/contrib --exclude=/.git --exclude=/Makefile)
if(ENABLE_NEK)
  list(APPEND _mirror_excludes --exclude=/test/tests/nek_ci)
endif()

add_custom_target(mirror_source ALL
  COMMAND rsync -rlpgo --update
          ${_mirror_excludes}
          ${CMAKE_SOURCE_DIR}/ ${CARDINAL_STAGE_DIR}/
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_SOURCE_DIR}/Makefile ${CARDINAL_STAGE_DIR}/Makefile.cardinal
  COMMENT "Mirroring Cardinal source into the build tree"
  VERBATIM)

# -----------------------------------------------------------------------------
# MOOSE (framework + modules) -- always required. Not a CMake project (MOOSE
# has its own, separate CMake port in progress upstream, which this
# deliberately does not depend on -- see cmake/PLAN.md, Non-goals), so its
# source is resolved with NO_BUILD: Cardinal's own Makefile compiles it
# directly, exactly as it does today.
# -----------------------------------------------------------------------------
cardinal_add_submodule_dependency(moose
  SUBMODULE_PATH contrib/moose
  SOURCE_DIR      ${CARDINAL_STAGE_DIR}/contrib/moose
  BINARY_DIR      ${CARDINAL_STAGE_DIR}/build/moose
  INSTALL_DIR     ${CARDINAL_INSTALL_DIR}
  MARKER_FILE     .git
  NO_BUILD
  NO_GIT_SUBMODULES)

# -----------------------------------------------------------------------------
# WASP/libMesh/PETSc: either a pre-built install (container; Phase 1/2) or
# built from source via MOOSE's own scripts (Phase 3). All three set, or all
# three unset -- never touches MOOSE's own build system beyond invoking
# these scripts exactly as BUILD/SETENV already document. See cmake/PLAN.md,
# "Phase 3".
# -----------------------------------------------------------------------------
set(WASP_DIR    "$ENV{WASP_DIR}"    CACHE PATH "Top-level WASP install (leave unset, along with LIBMESH_DIR/PETSC_DIR, to build all three from source)")
set(LIBMESH_DIR "$ENV{LIBMESH_DIR}" CACHE PATH "Top-level libMesh install (see WASP_DIR)")
set(PETSC_DIR   "$ENV{PETSC_DIR}"   CACHE PATH "Top-level PETSc install (see WASP_DIR)")
set(HDF5_ROOT   "$ENV{HDF5_ROOT}"   CACHE PATH "Top-level HDF5 install (default: derived from PETSC_DIR)")

if(WASP_DIR AND LIBMESH_DIR AND PETSC_DIR)
  set(CARDINAL_BUILD_MOOSE_PREREQS FALSE)
elseif(NOT WASP_DIR AND NOT LIBMESH_DIR AND NOT PETSC_DIR)
  set(CARDINAL_BUILD_MOOSE_PREREQS TRUE)
else()
  message(FATAL_ERROR
    "Cardinal: WASP_DIR/LIBMESH_DIR/PETSC_DIR must be either all set (use "
    "a pre-built install, e.g. the container) or all unset (build all "
    "three from source) -- got a mix "
    "(WASP_DIR='${WASP_DIR}' LIBMESH_DIR='${LIBMESH_DIR}' PETSC_DIR='${PETSC_DIR}').")
endif()

if(CARDINAL_BUILD_MOOSE_PREREQS)
  # ---------------------------------------------------------------------------
  # Tier 3: build PETSc, then libMesh, then WASP (independent) from source.
  # ---------------------------------------------------------------------------
  message(STATUS "Cardinal: WASP_DIR/LIBMESH_DIR/PETSC_DIR not set -- building all three from source")

  # No chicken-and-egg trick available here (libMesh doesn't exist yet, so
  # libmesh-config can't be asked anything) -- pick compilers directly
  # instead, matching what SETENV/RUNME already rely on for the
  # non-container path. See cmake/PLAN.md, Phase 3, "Compiler discovery".
  if(DEFINED ENV{CC})
    set(LIBMESH_CC $ENV{CC})
  else()
    find_program(LIBMESH_CC NAMES mpicc NO_CACHE)
  endif()
  if(DEFINED ENV{CXX})
    set(LIBMESH_CXX $ENV{CXX})
  else()
    find_program(LIBMESH_CXX NAMES mpicxx NO_CACHE)
  endif()
  if(DEFINED ENV{FC})
    set(LIBMESH_FC $ENV{FC})
  else()
    find_program(LIBMESH_FC NAMES mpif90 NO_CACHE)
  endif()

  foreach(_c LIBMESH_CC LIBMESH_CXX LIBMESH_FC)
    if(NOT ${_c})
      message(FATAL_ERROR
        "Cardinal: could not find a compiler for ${_c} -- set CC/CXX/FC in "
        "the environment, or make sure mpicc/mpicxx/mpif90 are on PATH.")
    endif()
  endforeach()

  message(STATUS "Cardinal: building PETSc/libMesh/WASP with CC=${LIBMESH_CC} CXX=${LIBMESH_CXX} FC=${LIBMESH_FC}")

  # These scripts call `make -jN` with an explicit numeric N internally, not
  # $(MAKE) -- an explicit -jN on a recursive make always starts its own
  # independent job pool rather than joining an inherited jobserver (the
  # same mechanism behind the earlier "-j128 forced in submake" surprise,
  # generalized). Since we won't patch MOOSE's scripts, there's no way to
  # make them jobserver-aware, so -- rather than guess a number -- require
  # an explicit choice. Defaults from CMake's own standard
  # CMAKE_BUILD_PARALLEL_LEVEL env var if that's set at configure time
  # (note it's only read once, at configure time -- a later `cmake --build
  # -jN` with a different N won't retroactively change this). See
  # cmake/PLAN.md, Phase 3, "Parallelism".
  set(MOOSE_BUILD_PARALLELISM "$ENV{CMAKE_BUILD_PARALLEL_LEVEL}" CACHE STRING "Parallel jobs (-j) for building PETSc/libMesh/WASP from source. Required when building from source (defaults from CMAKE_BUILD_PARALLEL_LEVEL if set in the environment).")
  if(NOT MOOSE_BUILD_PARALLELISM)
    message(FATAL_ERROR
      "Cardinal: building PETSc/libMesh/WASP from source requires "
      "-DMOOSE_BUILD_PARALLELISM=<N> (or CMAKE_BUILD_PARALLEL_LEVEL in the "
      "environment) -- these scripts can't share cmake --build's own -j. "
      "See cmake/PLAN.md, Phase 3, Parallelism.")
  endif()

  set(PETSC_ARCH "arch-moose" CACHE STRING "PETSc build arch (matches update_and_rebuild_petsc.sh/Makefile's own default)")

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(_moose_method dbg)
  else()
    set(_moose_method opt)
  endif()

  # Matches what each script installs to by default (Makefile:73-75) --
  # PETSc installs in place at $PETSC_DIR/$PETSC_ARCH (no PETSC_PREFIX set).
  set(_moose_dir     ${CARDINAL_STAGE_DIR}/contrib/moose)
  set(_moose_scripts ${_moose_dir}/scripts)
  # PETSC_DIR/LIBMESH_DIR/WASP_DIR deliberately stay plain (non-cache)
  # local overrides here, not written back with CACHE ... FORCE: the
  # empty-vs-set state of these three specific variables in the cache is
  # what the tier-1-vs-tier-3 branch above keys off of. Force-writing
  # them would make a from-source build look like a pre-built install
  # was given on the *next* reconfigure, permanently flipping into tier
  # 1 (and failing, since nothing was actually installed at those
  # computed paths as a "pre-built" install).
  set(PETSC_DIR   ${_moose_dir}/petsc)
  set(LIBMESH_DIR ${_moose_dir}/libmesh/installed)
  set(WASP_DIR    ${_moose_dir}/framework/contrib/wasp/install)
  # HDF5_ROOT carries no such meaning -- nothing branches on whether it
  # was user-set vs derived -- so FORCE it into the cache so ccmake
  # actually shows the value in use instead of the empty default from
  # its initial declaration above.
  set(HDF5_ROOT ${PETSC_DIR}/${PETSC_ARCH} CACHE PATH "Top-level HDF5 install (default: derived from PETSC_DIR)" FORCE)

  # These scripts' CONFIGURE_COMMAND/BUILD_COMMAND run as a plain custom
  # command (bash ${DEP_SCRIPT}, not the literal $(MAKE) token), so they
  # never get CMake's special jobserver-fd-preserving recipe treatment --
  # the inherited MAKEFLAGS text (e.g. "-j16 --jobserver-auth=3,4") still
  # leaks into their environment, but fds 3/4 no longer point at the real
  # jobserver pipe by the time it reaches them. PETSc's own libs rule
  # (lib/petsc/conf/rules_doc.mk) sees that stale "-j" in MAKEFLAGS,
  # assumes a real jobserver is already handing out tokens, and
  # deliberately adds none of its own -- silently building one file at a
  # time. Overriding MAKEFLAGS here to a clean "-jN" (no jobserver-auth
  # reference) makes GNU Make bootstrap its own fresh, working jobserver
  # for the invocation instead of trusting the broken inherited fds.
  set(_moose_makeflags MAKEFLAGS=-j${MOOSE_BUILD_PARALLELISM})

  # configure_petsc.sh hardcodes --download-openblas-make-options=
  # 'DYNAMIC_ARCH=1'; PETSc's own configure treats repeated --key=value
  # options as last-one-wins, and update_and_rebuild_petsc.sh appends our
  # SCRIPT_ARGS after its own hardcoded options, so this cleanly
  # overrides it without touching the MOOSE script.
  if(OPENBLAS_DYNAMIC_ARCH)
    set(_petsc_openblas_dynamic_arch 1)
  else()
    set(_petsc_openblas_dynamic_arch 0)
  endif()

  # Same story as OCCA/HYPRE above, but for PETSc's own --download-kokkos/
  # --download-kokkos-kernels: configure_petsc.sh never says --with-cuda
  # or --with-hip either way, so PETSc's configure auto-detects whatever
  # CUDA/HIP toolkit happens to be on this host and silently builds
  # GPU-enabled Kokkos. Tie this to the same ENABLE_CUDA/ENABLE_HIP
  # toggles used for NekRS/OCCA above, so one ccmake switch controls GPU
  # support consistently across the whole from-source build.
  if(ENABLE_CUDA)
    set(_petsc_with_cuda 1)
  else()
    set(_petsc_with_cuda 0)
  endif()
  if(ENABLE_HIP)
    set(_petsc_with_hip 1)
  else()
    set(_petsc_with_hip 0)
  endif()
  if(ENABLE_DPCPP)
    set(_petsc_with_sycl 1)
  else()
    set(_petsc_with_sycl 0)
  endif()

  cardinal_add_moose_prereq(petsc
    SCRIPT      ${_moose_scripts}/update_and_rebuild_petsc.sh
    SOURCE_DIR  ${_moose_dir}/petsc
    ENV         CC=${LIBMESH_CC} CXX=${LIBMESH_CXX} FC=${LIBMESH_FC}
                PETSC_ARCH=${PETSC_ARCH}
                MOOSE_JOBS=${MOOSE_BUILD_PARALLELISM}
                ${_moose_makeflags}
    SCRIPT_ARGS --download-openblas-make-options=DYNAMIC_ARCH=${_petsc_openblas_dynamic_arch}
                --with-cuda=${_petsc_with_cuda}
                --with-hip=${_petsc_with_hip}
                --with-sycl=${_petsc_with_sycl}
    DEPENDS     moose)

  # HPC login nodes commonly lack libtirpc-devel (no rpc/xdr.h) -- unlike
  # the container image, which bundles it. libMesh's configure hard-fails
  # on that ("XDR was not found, but --enable-xdr-required was
  # specified.") since update_and_rebuild_libmesh.sh always passes
  # --enable-xdr-required by default. doc/content/hpc_build_run_tips.md
  # documents the fix: build with XDR (legacy binary I/O; Cardinal
  # normally uses Exodus) disabled outright via these two flags.
  cardinal_add_moose_prereq(libmesh
    SCRIPT      ${_moose_scripts}/update_and_rebuild_libmesh.sh
    SOURCE_DIR  ${_moose_dir}/libmesh
    ENV         CC=${LIBMESH_CC} CXX=${LIBMESH_CXX} FC=${LIBMESH_FC}
                PETSC_DIR=${PETSC_DIR} PETSC_ARCH=${PETSC_ARCH}
                METHOD=${_moose_method}
                MOOSE_JOBS=${MOOSE_BUILD_PARALLELISM}
                ${_moose_makeflags}
    SCRIPT_ARGS --disable-xdr-required --disable-xdr
    DEPENDS     moose petsc)

  cardinal_add_moose_prereq(wasp
    SCRIPT     ${_moose_scripts}/update_and_rebuild_wasp.sh
    SOURCE_DIR ${_moose_dir}/framework/contrib/wasp
    ENV        CC=${LIBMESH_CC} CXX=${LIBMESH_CXX} FC=${LIBMESH_FC}
               MOOSE_JOBS=${MOOSE_BUILD_PARALLELISM}
               ${_moose_makeflags}
    DEPENDS    moose)

  # petsc's and wasp's *configure* steps (the full, non-`--fast` script) each
  # run `git submodule update --init --recursive <path>` against the same
  # shared MOOSE_DIR checkout. wasp only DEPENDS on moose, so with no
  # ordering between the two, ExternalProject can start both configure steps
  # at once -- and they race on MOOSE_DIR/.git/config:
  #   error: could not lock config file .git/config: File exists
  #   fatal: Failed to register url for submodule path 'framework/contrib/wasp'
  # Serialize just the configure steps (not the whole build) so PETSc's slow
  # build can still overlap with WASP's much faster one once both
  # submodules are fetched.
  add_dependencies(wasp-configure petsc-configure)
else()
  # ---------------------------------------------------------------------------
  # Tier 1: locate the pre-built install (container). Never builds these.
  # ---------------------------------------------------------------------------
  if(NOT HDF5_ROOT)
    set(HDF5_ROOT ${PETSC_DIR} CACHE PATH "Top-level HDF5 install (default: derived from PETSC_DIR)" FORCE)
  endif()

  if(NOT EXISTS ${LIBMESH_DIR}/bin/libmesh-config)
    message(FATAL_ERROR "Could not find ${LIBMESH_DIR}/bin/libmesh-config -- is LIBMESH_DIR (${LIBMESH_DIR}) correct?")
  endif()

  # Discover the exact compiler triple libMesh (and therefore MOOSE/Cardinal)
  # was built with. The Makefile normally derives this deep inside
  # framework/build.mk, which only runs once `make` starts -- too late for
  # our ExternalProject configure steps, which need it up front. See
  # cmake/PLAN.md, "Compiler discovery".
  execute_process(COMMAND ${LIBMESH_DIR}/bin/libmesh-config --cc
    OUTPUT_VARIABLE LIBMESH_CC OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND ${LIBMESH_DIR}/bin/libmesh-config --cxx
    OUTPUT_VARIABLE LIBMESH_CXX OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND ${LIBMESH_DIR}/bin/libmesh-config --fc
    OUTPUT_VARIABLE LIBMESH_FC OUTPUT_STRIP_TRAILING_WHITESPACE)

  foreach(_c LIBMESH_CC LIBMESH_CXX LIBMESH_FC)
    if(NOT ${_c})
      message(FATAL_ERROR "libmesh-config did not report a compiler for ${_c}")
    endif()
  endforeach()

  message(STATUS "Cardinal: compilers from libmesh-config -- CC=${LIBMESH_CC} CXX=${LIBMESH_CXX} FC=${LIBMESH_FC}")
endif()

# -----------------------------------------------------------------------------
# nuclear_data -- required whenever ENABLE_OPENMC is on (check_deps.mk); just
# a header (ADDITIONAL_INCLUDES in the Makefile), not a CMake project.
# -----------------------------------------------------------------------------
if(ENABLE_OPENMC)
  cardinal_add_submodule_dependency(nuclear_data
    SUBMODULE_PATH contrib/nuclear_data
    SOURCE_DIR      ${CARDINAL_STAGE_DIR}/contrib/nuclear_data
    BINARY_DIR      ${CARDINAL_STAGE_DIR}/build/nuclear_data
    INSTALL_DIR     ${CARDINAL_INSTALL_DIR}
    MARKER_FILE     .git
    NO_BUILD
    NO_GIT_SUBMODULES)
endif()

# -----------------------------------------------------------------------------
# NekRS (config/nekrs.mk)
# -----------------------------------------------------------------------------
if(ENABLE_NEK)
  # NekRS's own CMakeLists sets OCCA_ENABLE_CUDA/HIP/OPENCL/DPCPP/METAL to
  # ON via set(... CACHE BOOL ...) (CMakeLists.txt:143-147) -- ENABLE_CUDA/
  # ENABLE_HIP/ENABLE_OPENCL below only gate nekrs's own solver-level
  # compile definitions (CMakeLists.txt:287), they do NOT stop OCCA itself
  # from probing for and building against whatever CUDA/HIP toolkits
  # happen to be on PATH. On a bare HPC host with GPU toolkits present
  # (unlike the container, which has neither) that silently builds OCCA's
  # CUDA/HIP backends -- and, transitively, nekRS's own bundled
  # device-enabled HYPRE (cmake/hypre.cmake: `if(ENABLE_HYPRE_GPU AND
  # (OCCA_CUDA_ENABLED OR OCCA_HIP_ENABLED))`) -- even if that's not what
  # was asked for. Forward our own ENABLE_CUDA/HIP/OPENCL/DPCPP/METAL
  # cache variables (default OFF, toggle with ccmake) directly as OCCA's
  # own cache variables so a plain set(... CACHE ...) can't turn them
  # back on (that never overwrites a cache entry that already exists, and
  # -D on the command line always creates/wins one first).
  cardinal_add_submodule_dependency(nekrs
    SUBMODULE_PATH contrib/nekRS
    SOURCE_DIR      ${CARDINAL_STAGE_DIR}/contrib/nekRS
    BINARY_DIR      ${CARDINAL_STAGE_DIR}/build/nekrs
    INSTALL_DIR     ${CARDINAL_INSTALL_DIR}
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DCMAKE_C_COMPILER=${LIBMESH_CC}
      -DCMAKE_CXX_COMPILER=${LIBMESH_CXX}
      -DCMAKE_Fortran_COMPILER=${LIBMESH_FC}
      -DCMAKE_INSTALL_PREFIX=${CARDINAL_INSTALL_DIR}
      -DCMAKE_INSTALL_MESSAGE=LAZY
      "-DNEK5000_PPLIST=PARRSB DPROCMAP"
      -DOCCA_CXX=${LIBMESH_CC}
      "-DOCCA_CXXFLAGS=-O2 -ftree-vectorize -funroll-loops -march=native -mtune=native"
      -DENABLE_CUDA=${ENABLE_CUDA}
      -DENABLE_OPENCL=${ENABLE_OPENCL}
      -DENABLE_HIP=${ENABLE_HIP}
      -DENABLE_AMGX=0
      -DOCCA_ENABLE_CUDA=${ENABLE_CUDA}
      -DOCCA_ENABLE_HIP=${ENABLE_HIP}
      -DOCCA_ENABLE_OPENCL=${ENABLE_OPENCL}
      -DOCCA_ENABLE_DPCPP=${ENABLE_DPCPP}
      -DOCCA_ENABLE_METAL=${ENABLE_METAL})

  # NekRS's own CI test fixtures -- a separate submodule outside contrib/
  # (test/tests/nek_ci), not needed to build Cardinal itself, only to
  # run its NekRS-related tests. Not a CMake project, just test input
  # files -- same NO_BUILD/MARKER_FILE .git treatment as nuclear_data.
  cardinal_add_submodule_dependency(nek_ci
    SUBMODULE_PATH test/tests/nek_ci
    SOURCE_DIR      ${CARDINAL_STAGE_DIR}/test/tests/nek_ci
    BINARY_DIR      ${CARDINAL_STAGE_DIR}/build/nek_ci
    INSTALL_DIR     ${CARDINAL_INSTALL_DIR}
    MARKER_FILE     .git
    NO_BUILD
    NO_GIT_SUBMODULES)
endif()

# -----------------------------------------------------------------------------
# OpenMC (config/openmc.mk)
# -----------------------------------------------------------------------------
if(ENABLE_OPENMC)
  set(_openmc_vendored ON)
  if(EXISTS "$ENV{CONDA_PREFIX}/share/moose-compilers")
    set(_openmc_vendored OFF)
  endif()

  # OpenMC's own CMakeLists does find_package(LIBMESH) (pkg_check_modules
  # against CMAKE_PREFIX_PATH) -- in tier 1 libMesh always pre-exists at
  # configure time so no ordering is needed, but in tier 3 (Phase 3) it
  # doesn't exist until the libmesh target actually finishes building.
  set(_openmc_depends "")
  if(CARDINAL_BUILD_MOOSE_PREREQS)
    list(APPEND _openmc_depends libmesh)
  endif()

  cardinal_add_submodule_dependency(openmc
    SUBMODULE_PATH contrib/openmc
    SOURCE_DIR      ${CARDINAL_STAGE_DIR}/contrib/openmc
    BINARY_DIR      ${CARDINAL_STAGE_DIR}/build/openmc
    INSTALL_DIR     ${CARDINAL_INSTALL_DIR}
    DEPENDS         ${_openmc_depends}
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DOPENMC_USE_LIBMESH=ON
      -DOPENMC_USE_MPI=ON
      -DOPENMC_USE_DAGMC=OFF
      -DOPENMC_FORCE_VENDORED_LIBS=${_openmc_vendored}
      -DCMAKE_C_COMPILER=${LIBMESH_CC}
      -DCMAKE_CXX_COMPILER=${LIBMESH_CXX}
      -DCMAKE_Fortran_COMPILER=${LIBMESH_FC}
      -DCMAKE_PREFIX_PATH=${LIBMESH_DIR}
      -DHDF5_ROOT=${HDF5_ROOT}
      -DCMAKE_INSTALL_PREFIX=${CARDINAL_INSTALL_DIR}
      -DCMAKE_INSTALL_LIBDIR=${CARDINAL_INSTALL_DIR}/lib
      -DCMAKE_CXX_STANDARD=17
      "-DCMAKE_CXX_FLAGS=-Wno-error=reorder"
      -DCMAKE_INSTALL_MESSAGE=LAZY)
endif()

# -----------------------------------------------------------------------------
# Final step: hand off to Cardinal's existing, unmodified Makefile.
# -----------------------------------------------------------------------------
set(_cardinal_depends mirror_source moose)
if(CARDINAL_BUILD_MOOSE_PREREQS)
  list(APPEND _cardinal_depends wasp libmesh)  # libmesh pulls in petsc transitively
endif()
set(_make_enable_args
  ENABLE_NEK=$<IF:$<BOOL:${ENABLE_NEK}>,yes,no>
  ENABLE_OPENMC=$<IF:$<BOOL:${ENABLE_OPENMC}>,yes,no>
  ENABLE_DAGMC=$<IF:$<BOOL:${ENABLE_DAGMC}>,yes,no>
  ENABLE_DOUBLE_DOWN=$<IF:$<BOOL:${ENABLE_DOUBLE_DOWN}>,yes,no>)

if(ENABLE_NEK)
  list(APPEND _cardinal_depends nekrs nek_ci)
endif()
if(ENABLE_OPENMC)
  list(APPEND _cardinal_depends openmc nuclear_data)
endif()

add_custom_target(cardinal ALL
  # NEKRS_HOME/CONTRIB_INSTALL_DIR are computed paths we deliberately force
  # regardless of the ambient environment (check_nekrs.mk hard-requires
  # NEKRS_HOME == CONTRIB_INSTALL_DIR). OPENMC_CROSS_SECTIONS is left out of
  # this override list on purpose: it must come from whatever the user has
  # exported in the shell that invokes `cmake --build`/`make`, not get
  # frozen at CMake *configure* time via $ENV{...}.
  #
  # $(MAKE), not a literal `make`/`make -jN`: CMake's Makefiles generator
  # substitutes this with a recursive make invocation that shares the
  # invoking make's jobserver, so this step's parallelism is always exactly
  # whatever `-j` the user passed to `cmake --build`/`make` in this build
  # directory -- never a number this CMakeLists.txt invents itself, and
  # never more cores than were actually requested.
  COMMAND ${CMAKE_COMMAND} -E env
          NEKRS_HOME=${CARDINAL_INSTALL_DIR}
          CONTRIB_INSTALL_DIR=${CARDINAL_INSTALL_DIR}
          --
          $(MAKE) -f Makefile.cardinal ${_make_enable_args}
  WORKING_DIRECTORY ${CARDINAL_STAGE_DIR}
  DEPENDS ${_cardinal_depends}
  COMMENT "Building Cardinal via its own Makefile in ${CARDINAL_STAGE_DIR}"
  USES_TERMINAL
  VERBATIM)
