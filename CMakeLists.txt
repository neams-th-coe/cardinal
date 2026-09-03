cmake_minimum_required(VERSION 3.21)  # find_program(... NO_CACHE) used below
project(CardinalSuperbuild NONE)

# This is a *superbuild*: it never compiles Cardinal itself. It resolves and
# builds the third-party dependencies that have their own CMake sub-build
# (currently NekRS and OpenMC) and then hands off to Cardinal's existing,
# unmodified Makefile to compile the app.

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

# NekRS's own bundled HYPRE builds a CUDA/HIP device variant whenever
# ENABLE_CUDA/ENABLE_HIP is on (cmake/hypre.cmake:
# `if(ENABLE_HYPRE_GPU AND (OCCA_CUDA_ENABLED OR OCCA_HIP_ENABLED))`),
# defaulting ON regardless of caller. This can fail to build against a
# given GPU toolchain independent of anything Cardinal or this superbuild
# controls -- e.g. bundled HYPRE 2.32.0's CUDA device code doesn't build
# against CUDA >=13.3 (CCCL >=3.3 stopped transitively including
# thrust/tuple.h, fixed upstream in hypre-space/hypre#1547, well after
# 2.32.0 -- see doc/content/cmake_details.md), and Cardinal's own
# doc/content/hpc.md documents needing this off on Frontier (HIP) too,
# historically by hand-editing config/nekrs.mk's CMake flags. Expose it
# here instead so a superbuild user hitting the same class of problem can
# just pass -DENABLE_HYPRE_GPU=OFF at configure time.
option(ENABLE_HYPRE_GPU "Build NekRS's bundled HYPRE with GPU (CUDA/HIP) device support" ON)

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

# check_deps.mk silently downgrades ENABLE_DAGMC to no when ENABLE_OPENMC is
# off (DAGMC only matters as an OpenMC geometry backend) -- fail loudly at
# configure time instead, matching how ENABLE_DOUBLE_DOWN-without-ENABLE_
# DAGMC is handled just above, rather than silently building nothing.
if(ENABLE_DAGMC AND NOT ENABLE_OPENMC)
  message(FATAL_ERROR "ENABLE_DAGMC requires ENABLE_OPENMC")
endif()

if(ENABLE_DAGMC AND NOT ENABLE_DOUBLE_DOWN)
  message(WARNING
    "Cardinal: building with DAGMC but without Double-Down. For mesh-based "
    "geometries, Double-Down can yield speedups of ~100x and is highly "
    "recommended (-DENABLE_DOUBLE_DOWN=ON).")
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
#
# --exclude=/build --exclude=/install: Cardinal's own native Makefile
# defaults both to exactly these names directly under the source checkout
# (Makefile:69,76: CARDINAL_DIR/build, CARDINAL_DIR/install -- the same
# *_BUILDDIR/*_INSTALL_DIR paths this file's own dependency blocks are
# careful to match). Since CARDINAL_STAGE_DIR *is* the CMake build
# directory, a bare rsync of the whole source tree into it would, for
# anyone who has ever run (or runs concurrently) a native build in that
# same checkout, copy that leftover/live build/install tree wholesale on
# top of our own ExternalProject build trees living at those same relative
# paths -- silently corrupting them (observed directly: a stale
# build/openmc/CMakeCache.txt from an earlier native build clobbered a
# freshly-configured ExternalProject build/openmc here). Excluded
# unconditionally, not just when such a directory happens to exist.
#
# --exclude=/CMakeLists.txt --exclude=/cmake: this superbuild's own files
# (this file, and the helper modules under cmake/). Cardinal's native
# Makefile has no use for either -- they exist purely to drive this CMake
# configure -- so mirroring them into CARDINAL_STAGE_DIR (which, again, is
# the CMake build directory itself) would just leave a redundant, easily
# stale copy sitting next to CMake's own CMakeCache.txt/CMakeFiles for no
# purpose.
set(_mirror_excludes --exclude=/contrib --exclude=/.git --exclude=/Makefile --exclude=/build --exclude=/install --exclude=/CMakeLists.txt --exclude=/cmake)
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
# deliberately does not depend on), so its source is resolved with NO_BUILD:
# Cardinal's own Makefile compiles it directly, exactly as it does today.
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
# WASP/libMesh/PETSc: each independently either a pre-built install or built
# from source via MOOSE's own scripts -- never touches MOOSE's own build
# system beyond invoking these scripts exactly as BUILD/SETENV already
# document.
#
# The three are independent except for one real constraint: a pre-built
# LIBMESH_DIR was compiled against some specific PETSc, and there's no way
# to discover which one from LIBMESH_DIR alone -- so LIBMESH_DIR requires
# PETSC_DIR to also be given. WASP has no build or link dependency on either
# of the other two (it's a standalone CMake+Ninja tool), so it's fully
# independent; e.g. PETSC_DIR alone can be given (a system/module-provided
# PETSc -- by far the slowest, most painful of the three to build from
# source) while libMesh and WASP are still built from scratch.
# -----------------------------------------------------------------------------
set(WASP_DIR    "$ENV{WASP_DIR}"    CACHE PATH "Top-level WASP install (leave unset to build from source)")
set(LIBMESH_DIR "$ENV{LIBMESH_DIR}" CACHE PATH "Top-level libMesh install (leave unset to build from source; requires PETSC_DIR also set)")
set(PETSC_DIR   "$ENV{PETSC_DIR}"   CACHE PATH "Top-level PETSc install (leave unset to build from source)")
set(HDF5_ROOT   "$ENV{HDF5_ROOT}"   CACHE PATH "Top-level HDF5 install (default: derived from PETSC_DIR)")

if(LIBMESH_DIR AND NOT PETSC_DIR)
  message(FATAL_ERROR
    "Cardinal: LIBMESH_DIR is set but PETSC_DIR is not -- a pre-built "
    "libMesh was compiled against some specific PETSc, and there's no way "
    "to discover which one from LIBMESH_DIR alone. Set PETSC_DIR too.")
endif()

if(NOT PETSC_DIR)
  set(CARDINAL_BUILD_PETSC TRUE)
else()
  set(CARDINAL_BUILD_PETSC FALSE)
endif()
if(NOT LIBMESH_DIR)
  set(CARDINAL_BUILD_LIBMESH TRUE)
else()
  set(CARDINAL_BUILD_LIBMESH FALSE)
endif()
if(NOT WASP_DIR)
  set(CARDINAL_BUILD_WASP TRUE)
else()
  set(CARDINAL_BUILD_WASP FALSE)
endif()

# Compiler discovery: if libMesh is pre-built, ask it directly (most
# authoritative -- this is the same toolchain MOOSE/Cardinal itself needs to
# use). Otherwise, libMesh doesn't exist yet so there's no chicken-and-egg
# trick available -- pick compilers directly from the environment instead,
# matching what SETENV/RUNME already rely on for the non-container path.
# Trust that the user knows what they're doing if PETSC_DIR is externally
# provided (e.g. a module load already implies a compatible compiler)
# rather than trying to reverse-engineer PETSc's own build.
if(NOT CARDINAL_BUILD_LIBMESH)
  if(NOT EXISTS ${LIBMESH_DIR}/bin/libmesh-config)
    message(FATAL_ERROR "Could not find ${LIBMESH_DIR}/bin/libmesh-config -- is LIBMESH_DIR (${LIBMESH_DIR}) correct?")
  endif()

  # The Makefile normally derives this deep inside framework/build.mk,
  # which only runs once `make` starts -- too late for our ExternalProject
  # configure steps, which need it up front.
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
else()
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

  message(STATUS "Cardinal: building libMesh with CC=${LIBMESH_CC} CXX=${LIBMESH_CXX} FC=${LIBMESH_FC}")
endif()

# Cardinal's own Makefile (Makefile:322-324: LIBMESH_CC_LIST := $(subst
# $(space),;,$(libmesh_CC)), etc) converts these to a semicolon-separated
# CMake list before handing them to any CMake-based dependency's own
# configure as CMAKE_C_COMPILER/CMAKE_CXX_COMPILER/CMAKE_Fortran_COMPILER.
# CMake accepts a semicolon-list value for these variables, treating
# entries after the first as required compiler arguments -- a bare string
# with embedded spaces (e.g. a compiler wrapper reported as "ccache
# mpicc", or a compound CC/CXX/FC override) would otherwise be misread as
# a single, invalid executable path. Only used for CMAKE_ARGS below, not
# for OCCA_CXX or the plain CC/CXX/FC exports to the PETSc/libMesh/WASP
# scripts further down -- config/nekrs.mk's own -DOCCA_CXX and Makefile:
# 262-264's own `export CC := ...` etc likewise pass these raw,
# unconverted. $<SEMICOLON>, not a literal ';': the result is itself one
# CMAKE_ARGS list entry threaded through ExternalProject_Add -- a literal
# ';' here would instead split it into two separate CMAKE_ARGS entries
# (see the CMAKE_INSTALL_RPATH note in the MOAB block below for the same
# issue).
string(REPLACE " " "$<SEMICOLON>" LIBMESH_CC_LIST  "${LIBMESH_CC}")
string(REPLACE " " "$<SEMICOLON>" LIBMESH_CXX_LIST "${LIBMESH_CXX}")
string(REPLACE " " "$<SEMICOLON>" LIBMESH_FC_LIST  "${LIBMESH_FC}")

if(CARDINAL_BUILD_PETSC OR CARDINAL_BUILD_LIBMESH OR CARDINAL_BUILD_WASP)
  set(_moose_dir     ${CARDINAL_STAGE_DIR}/contrib/moose)
  set(_moose_scripts ${_moose_dir}/scripts)

  # These scripts call `make -jN` with an explicit numeric N internally, not
  # $(MAKE) -- an explicit -jN on a recursive make always starts its own
  # independent job pool rather than joining an inherited jobserver (the
  # same mechanism behind the earlier "-j128 forced in submake" surprise,
  # generalized). Since we won't patch MOOSE's scripts, there's no way to
  # make them jobserver-aware, so some concrete number is unavoidable here
  # (unlike the final "cardinal" step below, which shares cmake --build's
  # own jobserver and so never needs to guess). Defaults from CMake's own
  # standard CMAKE_BUILD_PARALLEL_LEVEL env var if that's set at configure
  # time (note it's only read once, at configure time -- a later
  # `cmake --build -jN` with a different N won't retroactively change
  # this); if that's not set either, see the min(available cores, 16)
  # default below.
  set(MOOSE_BUILD_PARALLELISM "$ENV{CMAKE_BUILD_PARALLEL_LEVEL}" CACHE STRING "Parallel jobs (-j) for building PETSc/libMesh/WASP from source. Defaults from CMAKE_BUILD_PARALLEL_LEVEL if set in the environment, else min(available cores, 16).")
  if(NOT MOOSE_BUILD_PARALLELISM)
    # Unlike the final "cardinal" build step (which shares cmake --build's
    # own jobserver via $(MAKE) and so must never guess a number), these
    # scripts genuinely can't share that jobserver -- some concrete number
    # is unavoidable. Default to something reasonable (capped at 16 so an
    # unattended default doesn't grab an entire shared/login-node machine)
    # rather than hard-failing; it's just a CACHE STRING, so -DMOOSE_BUILD_
    # PARALLELISM=<N> or editing it directly in ccmake overrides it same as
    # any other cache variable.
    cmake_host_system_information(RESULT _ncpu QUERY NUMBER_OF_LOGICAL_CORES)
    if(_ncpu GREATER 16)
      set(_default_moose_build_parallelism 16)
    else()
      set(_default_moose_build_parallelism ${_ncpu})
    endif()
    set(MOOSE_BUILD_PARALLELISM ${_default_moose_build_parallelism} CACHE STRING "Parallel jobs (-j) for building PETSc/libMesh/WASP from source. Defaults from CMAKE_BUILD_PARALLEL_LEVEL if set in the environment, else min(available cores, 16)." FORCE)
    message(WARNING
      "Cardinal: MOOSE_BUILD_PARALLELISM not set (nor CMAKE_BUILD_PARALLEL_LEVEL "
      "in the environment) -- defaulting to ${MOOSE_BUILD_PARALLELISM} "
      "(min(available cores, 16)); these scripts can't share cmake --build's "
      "own -j. Set -DMOOSE_BUILD_PARALLELISM=<N> (or edit it in ccmake) to "
      "override.")
  endif()

  # Escape hatches for anything not already exposed as its own CMake option
  # above (ENABLE_CUDA/ENABLE_HIP/..., OPENBLAS_DYNAMIC_ARCH, etc) -- forwarded
  # verbatim, after our own hard-coded args, to each script's underlying
  # configure (PETSc's own `configure`, libMesh's `configure`, WASP's `cmake`;
  # all three already accept and forward arbitrary extra arguments this way).
  # Since PETSc's/libMesh's configure treat repeated --key=value options as
  # last-one-wins, anything set here can also override one of our hard-coded
  # args below rather than just add to them.
  set(PETSC_SCRIPT_ARGS   "" CACHE STRING "Extra arguments appended to update_and_rebuild_petsc.sh's full (non-\"--fast\") configure run.")
  set(LIBMESH_SCRIPT_ARGS "" CACHE STRING "Extra arguments appended to update_and_rebuild_libmesh.sh's full (non-\"--fast\") configure run.")
  set(WASP_SCRIPT_ARGS    "" CACHE STRING "Extra arguments appended to update_and_rebuild_wasp.sh's full (non-\"--fast\") configure run.")
  separate_arguments(PETSC_SCRIPT_ARGS)
  separate_arguments(LIBMESH_SCRIPT_ARGS)
  separate_arguments(WASP_SCRIPT_ARGS)

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

  if(CARDINAL_BUILD_PETSC)
    set(PETSC_ARCH "arch-moose" CACHE STRING "PETSc build arch (matches update_and_rebuild_petsc.sh/Makefile's own default)")
    # Matches what the script installs to by default (Makefile:73-75) --
    # PETSc installs in place at $PETSC_DIR/$PETSC_ARCH (no PETSC_PREFIX set).
    # Plain set, not CACHE FORCE -- see the note at the end of this block.
    set(PETSC_DIR ${_moose_dir}/petsc)

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

    # Same story as OCCA/HYPRE elsewhere, but for PETSc's own
    # --download-kokkos/--download-kokkos-kernels: configure_petsc.sh never
    # says --with-cuda or --with-hip either way, so PETSc's configure
    # auto-detects whatever CUDA/HIP toolkit happens to be on this host and
    # silently builds GPU-enabled Kokkos. Tie this to the same ENABLE_CUDA/
    # ENABLE_HIP toggles used for NekRS/OCCA, so one ccmake switch controls
    # GPU support consistently across the whole from-source build.
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
                  ${PETSC_SCRIPT_ARGS}
      DEPENDS     moose)
  endif()

  if(CARDINAL_BUILD_LIBMESH)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
      set(_moose_method dbg)
    else()
      set(_moose_method opt)
    endif()

    set(_libmesh_depends moose)
    if(CARDINAL_BUILD_PETSC)
      list(APPEND _libmesh_depends petsc)
    endif()

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
                  ${LIBMESH_SCRIPT_ARGS}
      DEPENDS     ${_libmesh_depends})

    # Plain set, not CACHE FORCE -- see the note below.
    set(LIBMESH_DIR ${_moose_dir}/libmesh/installed)
  endif()

  if(CARDINAL_BUILD_WASP)
    cardinal_add_moose_prereq(wasp
      SCRIPT     ${_moose_scripts}/update_and_rebuild_wasp.sh
      SOURCE_DIR ${_moose_dir}/framework/contrib/wasp
      ENV        CC=${LIBMESH_CC} CXX=${LIBMESH_CXX} FC=${LIBMESH_FC}
                 MOOSE_JOBS=${MOOSE_BUILD_PARALLELISM}
                 ${_moose_makeflags}
      SCRIPT_ARGS ${WASP_SCRIPT_ARGS}
      DEPENDS    moose)

    # Plain set, not CACHE FORCE -- see the note below.
    set(WASP_DIR ${_moose_dir}/framework/contrib/wasp/install)
  endif()

  # petsc's and wasp's *configure* steps (the full, non-`--fast` script) each
  # run `git submodule update --init --recursive <path>` against the same
  # shared MOOSE_DIR checkout. With no ordering between the two,
  # ExternalProject can start both configure steps at once when we're
  # building both from source -- and they race on MOOSE_DIR/.git/config:
  #   error: could not lock config file .git/config: File exists
  #   fatal: Failed to register url for submodule path 'framework/contrib/wasp'
  # Serialize just the configure steps (not the whole build) so PETSc's slow
  # build can still overlap with WASP's much faster one once both
  # submodules are fetched.
  if(CARDINAL_BUILD_PETSC AND CARDINAL_BUILD_WASP)
    add_dependencies(wasp-configure petsc-configure)
  endif()

  # PETSC_DIR/LIBMESH_DIR/WASP_DIR (whichever were computed above)
  # deliberately stay plain (non-cache) local overrides, not written back
  # with CACHE ... FORCE: the empty-vs-set state of each in the cache is
  # exactly what this whole section keys off of. Force-writing one would
  # make a from-source build look like a pre-built install was given for
  # it on the *next* reconfigure, permanently flipping it to "pre-built"
  # (and failing, since nothing was actually installed at that computed
  # path as a genuine pre-built install).
endif()

# HDF5_ROOT: derived from PETSC_DIR either way, but the layout differs -- our
# own from-source PETSc uses $PETSC_DIR/$PETSC_ARCH (no PETSC_PREFIX set),
# while an externally-provided PETSC_DIR is a flat, complete prefix with no
# arch subdirectory (matching how libMesh's own configure treats PETSC_DIR
# when PETSC_ARCH is left unset). FORCE it into the cache so ccmake shows the
# value actually in use; safe here (unlike PETSC_DIR/LIBMESH_DIR/WASP_DIR
# above) since nothing branches on whether HDF5_ROOT itself was user-set vs.
# derived.
#
# Deliberately placed *outside* (after) the CARDINAL_BUILD_PETSC/LIBMESH/WASP
# block above, not inside it: this used to live inside that block, which
# meant it silently never ran at all -- HDF5_ROOT stayed permanently empty in
# the cache -- whenever PETSc/libMesh/WASP were all pre-built (i.e. the
# common container case, where that whole block is skipped). MOAB's and
# OpenMC's own CMAKE_ARGS forward this value explicitly regardless (so their
# own sub-builds were never affected), but nothing forwarded the *computed*
# value on to Cardinal's own native Makefile step -- it fell back to
# whatever HDF5_ROOT happened to already be in the ambient shell environment
# invoking `cmake --build`/`make`, silently wrong (or empty) if the user
# hadn't separately exported it themselves. Found live: a user reported
# needing to export HDF5_ROOT manually for it to "make it to the moose
# sub-dependencies" despite this superbuild computing it correctly.
if(NOT HDF5_ROOT)
  if(CARDINAL_BUILD_PETSC)
    set(HDF5_ROOT ${PETSC_DIR}/${PETSC_ARCH} CACHE PATH "Top-level HDF5 install (default: derived from PETSC_DIR)" FORCE)
  else()
    set(HDF5_ROOT ${PETSC_DIR} CACHE PATH "Top-level HDF5 install (default: derived from PETSC_DIR)" FORCE)
  endif()
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
  # NekRS's own CMakeLists.txt defaults some of its own OCCA_ENABLE_*
  # cache variables to ON regardless of caller -- OCCA_ENABLE_CUDA/HIP/
  # DPCPP, via set(... CACHE BOOL ...) (CMakeLists.txt:143-145;
  # OCCA_ENABLE_OPENCL/METAL default OFF there already, :146-147).
  # Separately, our own ENABLE_CUDA/ENABLE_HIP/ENABLE_OPENCL below
  # (matching config/nekrs.mk's own -D flags exactly) do NOT stop OCCA
  # itself from probing for and building against whatever CUDA/HIP
  # toolkits happen to be on PATH -- OCCA's actual backend build is gated
  # on its own downstream-computed OCCA_CUDA_ENABLED/OCCA_HIP_ENABLED/etc,
  # not on these input flags. On a bare HPC host with GPU toolkits present
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
      -DCMAKE_C_COMPILER=${LIBMESH_CC_LIST}
      -DCMAKE_CXX_COMPILER=${LIBMESH_CXX_LIST}
      -DCMAKE_Fortran_COMPILER=${LIBMESH_FC_LIST}
      -DCMAKE_INSTALL_PREFIX=${CARDINAL_INSTALL_DIR}
      -DCMAKE_INSTALL_MESSAGE=LAZY
      "-DNEK5000_PPLIST=PARRSB DPROCMAP"
      -DOCCA_CXX=${LIBMESH_CC}
      "-DOCCA_CXXFLAGS=-O2 -ftree-vectorize -funroll-loops -march=native -mtune=native"
      -DENABLE_CUDA=${ENABLE_CUDA}
      -DENABLE_OPENCL=${ENABLE_OPENCL}
      -DENABLE_HIP=${ENABLE_HIP}
      -DENABLE_AMGX=0
      -DENABLE_HYPRE_GPU=${ENABLE_HYPRE_GPU}
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
# DAGMC/MOAB/Embree/double-down (config/moab.mk, embree.mk, double_down.mk,
# dagmc.mk) -- CAD/mesh-based geometry support, consumed by OpenMC (below)
# via -DDAGMC_DIR. Same pattern as NekRS/OpenMC above: each gets its own
# independent CMake sub-build via cardinal_add_submodule_dependency (not
# NO_BUILD), with CMAKE_ARGS matching config/*.mk's own cmake invocations
# exactly, and BINARY_DIR/INSTALL_DIR matching the *_BUILDDIR/*_INSTALL_DIR
# paths those *.mk files themselves compute (Makefile:136-146) so Cardinal's
# own build_moab/build_embree/build_doubledown/build_dagmc rules -- run
# unconditionally as part of the final `make -f Makefile.cardinal` step --
# find our already-configured build trees and just do a cheap, idempotent
# `make install` rather than reconfiguring from scratch.
#
# Build order mirrors the *.mk files' own prerequisite chain (Makefile:281-
# 314): MOAB and Embree are independent of each other; double-down needs
# both; DAGMC needs MOAB always, and additionally double-down/Embree only
# when ENABLE_DOUBLE_DOWN is on (check_deps.mk only requires double-down/
# Embree in that case -- DAGMC can build without ray-tracing acceleration).
# -----------------------------------------------------------------------------
if(ENABLE_DAGMC)
  # Matches the Makefile's own default (Makefile:88): EIGEN3_DIR ?=
  # $(LIBMESH_DIR)/include. Only relevant when DAGMC is on (MOAB/DAGMC are
  # the only consumers), so declared here rather than alongside HDF5_ROOT
  # above.
  set(EIGEN3_DIR "$ENV{EIGEN3_DIR}" CACHE PATH "Top-level eigen3 dir (should contain FindEigen3.cmake); default: derived from LIBMESH_DIR")
  if(NOT EIGEN3_DIR)
    set(EIGEN3_DIR ${LIBMESH_DIR}/include CACHE PATH "Top-level eigen3 dir (should contain FindEigen3.cmake); default: derived from LIBMESH_DIR" FORCE)
  endif()

  cardinal_add_submodule_dependency(moab
    SUBMODULE_PATH contrib/moab
    SOURCE_DIR      ${CARDINAL_STAGE_DIR}/contrib/moab
    BINARY_DIR      ${CARDINAL_STAGE_DIR}/build/moab
    INSTALL_DIR     ${CARDINAL_INSTALL_DIR}
    CMAKE_ARGS
      -DENABLE_MPI=ON
      -DENABLE_HDF5=ON
      -DENABLE_TESTING=OFF
      -DENABLE_BLASLAPACK=OFF
      -DEIGEN3_DIR=${EIGEN3_DIR}
      -DHDF5_DIR=${HDF5_ROOT}
      # -DHDF5_DIR alone (moab.mk's own flag, matched above) isn't enough:
      # MOAB's CMakeLists.txt calls plain `find_package(HDF5 REQUIRED
      # COMPONENTS C HL)`, resolved in MODULE mode by CMake's own bundled
      # FindHDF5.cmake -- that module doesn't consult HDF5_DIR at all, only
      # HDF5_ROOT (as a CMake variable/cache entry, falling back to
      # $ENV{HDF5_ROOT} only if that's unset -- confirmed by reading
      # FindHDF5.cmake directly). Cardinal's own native moab.mk gets this
      # for free because the Makefile itself `export HDF5_ROOT`s (Makefile:
      # 93-95) before config/moab.mk's cmake invocation ever runs; our own
      # independent ExternalProject_Add sub-build has no such ambient
      # export to inherit, so it must be passed explicitly. Found live:
      # MOAB's own configure failed with "Could NOT find HDF5" as soon as
      # HDF5_ROOT wasn't already exported in the invoking shell -- i.e. this
      # superbuild's own carefully-computed HDF5_ROOT (above) wasn't
      # actually reaching MOAB's configure at all without this.
      -DHDF5_ROOT=${HDF5_ROOT}
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DCMAKE_C_COMPILER=${LIBMESH_CC_LIST}
      -DCMAKE_CXX_COMPILER=${LIBMESH_CXX_LIST}
      -DCMAKE_Fortran_COMPILER=${LIBMESH_FC_LIST}
      -DCMAKE_PREFIX_PATH=${LIBMESH_DIR}
      -DCMAKE_INSTALL_PREFIX=${CARDINAL_INSTALL_DIR}
      # A real semicolon-separated CMake list value threaded through
      # ExternalProject_Add's CMAKE_ARGS needs $<SEMICOLON> -- a bare ';'
      # here would just split this into two separate CMAKE_ARGS entries.
      -DCMAKE_INSTALL_RPATH=${HDF5_ROOT}/lib$<SEMICOLON>${CARDINAL_INSTALL_DIR}/lib
      -DCMAKE_INSTALL_LIBDIR=lib
      -DCMAKE_INSTALL_MESSAGE=LAZY)

  # Works around a real bug in MOAB's own exported CMake config -- see
  # CardinalFixupMoabConfig.cmake for the full explanation. Runs after
  # moab's own install step (STEP_TARGETS above includes "install", giving
  # us the moab-install target to hang this off of) and before anything
  # that consumes MOABConfig.cmake, i.e. DAGMC always, and double-down when
  # enabled.
  #
  # Patches *two* copies, not just the installed one: the final "build
  # cardinal" step below hands off to Cardinal's own native Makefile,
  # which -- entirely independently of everything above -- always redoes
  # its own build_moab (`make -C $(MOAB_BUILDDIR) install`) on every
  # invocation (build_moab is a .PHONY target with no staleness check of
  # its own; this is normal, pre-existing behavior of Cardinal's Makefile,
  # unrelated to this superbuild). That `make install` blindly re-copies
  # MOAB's own build-tree MOABConfig.cmake -- the un-patched template CMake
  # generates during MOAB's *configure* step -- over our patched installed
  # copy, silently reverting the fix (observed directly). Patching that
  # build-tree template too means every later re-install, ours or
  # Cardinal's native one, keeps re-copying an already-fixed file instead
  # of reverting it: self-healing rather than a one-time patch.
  add_custom_target(moab_fixup
    COMMAND ${CMAKE_COMMAND}
            -DMOAB_CONFIG=${CARDINAL_STAGE_DIR}/build/moab/MOABConfig.cmake
            -P ${CMAKE_SOURCE_DIR}/cmake/CardinalFixupMoabConfig.cmake
    COMMAND ${CMAKE_COMMAND}
            -DMOAB_CONFIG=${CARDINAL_INSTALL_DIR}/lib/cmake/MOAB/MOABConfig.cmake
            -P ${CMAKE_SOURCE_DIR}/cmake/CardinalFixupMoabConfig.cmake
    DEPENDS moab-install
    COMMENT "Cardinal: patching MOABConfig.cmake (CMP0004 workaround)"
    VERBATIM)

  if(ENABLE_DOUBLE_DOWN)
    cardinal_add_submodule_dependency(embree
      SUBMODULE_PATH contrib/embree
      SOURCE_DIR      ${CARDINAL_STAGE_DIR}/contrib/embree
      BINARY_DIR      ${CARDINAL_STAGE_DIR}/build/embree
      INSTALL_DIR     ${CARDINAL_INSTALL_DIR}
      CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_C_COMPILER=${LIBMESH_CC_LIST}
        -DCMAKE_CXX_COMPILER=${LIBMESH_CXX_LIST}
        -DCMAKE_Fortran_COMPILER=${LIBMESH_FC_LIST}
        -DCMAKE_PREFIX_PATH=${LIBMESH_DIR}
        -DCMAKE_INSTALL_PREFIX=${CARDINAL_INSTALL_DIR}
        -DCMAKE_INSTALL_LIBDIR=lib
        -DCMAKE_INSTALL_MESSAGE=LAZY
        -DEMBREE_TUTORIALS=OFF
        -DEMBREE_TUTORIALS_GLFW=OFF
        -DEMBREE_TASKING_SYSTEM=INTERNAL
        -DEMBREE_IGNORE_CMAKE_CXX_FLAGS=OFF)

    cardinal_add_submodule_dependency(doubledown
      SUBMODULE_PATH contrib/double-down
      SOURCE_DIR      ${CARDINAL_STAGE_DIR}/contrib/double-down
      BINARY_DIR      ${CARDINAL_STAGE_DIR}/build/double-down
      INSTALL_DIR     ${CARDINAL_INSTALL_DIR}
      DEPENDS         moab moab_fixup embree
      CMAKE_ARGS
        -DEMBREE_DIR=${CARDINAL_INSTALL_DIR}
        -DMOAB_DIR=${CARDINAL_INSTALL_DIR}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_C_COMPILER=${LIBMESH_CC_LIST}
        -DCMAKE_CXX_COMPILER=${LIBMESH_CXX_LIST}
        -DCMAKE_Fortran_COMPILER=${LIBMESH_FC_LIST}
        -DCMAKE_PREFIX_PATH=${LIBMESH_DIR}
        -DCMAKE_INSTALL_PREFIX=${CARDINAL_INSTALL_DIR}
        -DCMAKE_INSTALL_LIBDIR=lib
        -DCMAKE_INSTALL_MESSAGE=LAZY)
  endif()

  set(_dagmc_depends moab moab_fixup)
  if(ENABLE_DOUBLE_DOWN)
    list(APPEND _dagmc_depends doubledown embree)
  endif()

  # DOUBLE_DOWN/Eigen3_DIR/MOAB_DIR spellings below (as opposed to the
  # ENABLE_DOUBLE_DOWN/EIGEN3_DIR/CARDINAL_INSTALL_DIR vars they're forwarded
  # from) match dagmc.mk's own -D flags exactly (Cardinal's own DAGMC
  # CMakeLists.txt expects this exact casing).
  cardinal_add_submodule_dependency(dagmc
    SUBMODULE_PATH contrib/DAGMC
    SOURCE_DIR      ${CARDINAL_STAGE_DIR}/contrib/DAGMC
    BINARY_DIR      ${CARDINAL_STAGE_DIR}/build/DAGMC
    INSTALL_DIR     ${CARDINAL_INSTALL_DIR}
    DEPENDS         ${_dagmc_depends}
    CMAKE_ARGS
      -DBUILD_TALLY=OFF
      -DBUILD_TESTS=OFF
      -DBUILD_CI_TESTS=OFF
      -DBUILD_STATIC_LIBS=OFF
      -DBUILD_STATIC_EXE=OFF
      -DBUILD_UWUW=OFF
      -DENABLE_NETCDF=ON
      -DDOUBLE_DOWN=${ENABLE_DOUBLE_DOWN}
      -DDOUBLE_DOWN_DIR=${CARDINAL_INSTALL_DIR}
      -DEigen3_DIR=${EIGEN3_DIR}
      -DMOAB_DIR=${CARDINAL_INSTALL_DIR}
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DCMAKE_C_COMPILER=${LIBMESH_CC_LIST}
      -DCMAKE_CXX_COMPILER=${LIBMESH_CXX_LIST}
      -DCMAKE_Fortran_COMPILER=${LIBMESH_FC_LIST}
      -DCMAKE_PREFIX_PATH=${LIBMESH_DIR}
      -DCMAKE_INSTALL_PREFIX=${CARDINAL_INSTALL_DIR}
      -DCMAKE_INSTALL_LIBDIR=lib
      -DCMAKE_INSTALL_MESSAGE=LAZY)
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
  # against CMAKE_PREFIX_PATH) -- if libMesh is pre-built it always
  # pre-exists at configure time so no ordering is needed, but if we're
  # building it ourselves it doesn't exist until the libmesh target
  # actually finishes building. Same story for DAGMC: only needed once it
  # actually finishes building, and only when ENABLE_DAGMC is on.
  set(_openmc_depends "")
  if(CARDINAL_BUILD_LIBMESH)
    list(APPEND _openmc_depends libmesh)
  endif()
  if(ENABLE_DAGMC)
    list(APPEND _openmc_depends dagmc)
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
      -DOPENMC_USE_DAGMC=${ENABLE_DAGMC}
      -DDAGMC_DIR=${CARDINAL_INSTALL_DIR}
      -DOPENMC_FORCE_VENDORED_LIBS=${_openmc_vendored}
      -DCMAKE_C_COMPILER=${LIBMESH_CC_LIST}
      -DCMAKE_CXX_COMPILER=${LIBMESH_CXX_LIST}
      -DCMAKE_Fortran_COMPILER=${LIBMESH_FC_LIST}
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
if(CARDINAL_BUILD_WASP)
  list(APPEND _cardinal_depends wasp)
endif()
if(CARDINAL_BUILD_LIBMESH)
  list(APPEND _cardinal_depends libmesh)  # pulls in petsc transitively, if we're building that too
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
# dagmc's own DEPENDS (moab, and doubledown/embree when ENABLE_DOUBLE_DOWN)
# already chains transitively through openmc's DEPENDS above -- listed
# explicitly anyway, matching this file's style of spelling out the full
# dependency set for the final step rather than relying on a reader to
# trace it through openmc's own DEPENDS.
if(ENABLE_DAGMC)
  list(APPEND _cardinal_depends dagmc)
endif()

add_custom_target(cardinal ALL
  # NEKRS_HOME/CONTRIB_INSTALL_DIR/HDF5_ROOT/PETSC_DIR/LIBMESH_DIR/WASP_DIR
  # are computed paths we deliberately force regardless of the ambient
  # environment (check_nekrs.mk hard-requires NEKRS_HOME ==
  # CONTRIB_INSTALL_DIR). Without forcing PETSC_DIR/LIBMESH_DIR/WASP_DIR/
  # HDF5_ROOT here too, Cardinal's own native Makefile step falls back to
  # whatever each happens to already be in the ambient shell environment
  # invoking `cmake --build`/`make` -- and when unset there, to Cardinal's
  # own from-source-layout defaults (Makefile:73,75, moose.mk:94:
  # PETSC_DIR ?= $(MOOSE_DIR)/petsc, LIBMESH_DIR ?= $(MOOSE_DIR)/libmesh/
  # installed, WASP_DIR ?= $(MOOSE_DIR)/framework/contrib/wasp/install),
  # silently pointing at nothing whether the actual (pre-built or
  # from-source) install is really there or not. This went unnoticed for a
  # while because a real ambient environment (e.g. the moose-dev
  # container's own /opt/* variables) happens to make it work by accident
  # -- surfaced only by testing with those unset and given purely via -D
  # instead: `make: *** No rule to make target '.../contrib/moose/petsc/
  # lib/petsc/conf/petscvariables'` despite -DPETSC_DIR=/opt/petsc having
  # been given at configure time.
  #
  # OPENMC_CROSS_SECTIONS is left out of this override list on purpose: it
  # must come from whatever the user has exported in the shell that invokes
  # `cmake --build`/`make`, not get frozen at CMake *configure* time via
  # $ENV{...}.
  #
  # $(MAKE), not a literal `make`/`make -jN`: CMake's Makefiles generator
  # substitutes this with a recursive make invocation that shares the
  # invoking make's jobserver, so this step's parallelism is always exactly
  # whatever `-j` the user passed to `cmake --build`/`make` in this build
  # directory -- never a number this CMakeLists.txt invents itself, and
  # never more cores than were actually requested.
  #
  # _make_enable_args go through this same -E env, as environment variables,
  # rather than as trailing `make VAR=value` command-line arguments: GNU
  # Make gives a command-line-set variable unconditional priority over any
  # plain `VAR := value` assignment the makefile itself makes later --
  # only `override VAR := value` can beat it, which Cardinal's Makefile
  # doesn't use. Cardinal's own Makefile relies on exactly that freedom:
  # check_deps.mk/the Makefile itself canonicalize ENABLE_DAGMC/
  # ENABLE_DOUBLE_DOWN from yes/no into ON/OFF (and, when OpenMC is off,
  # silently downgrade ENABLE_DAGMC to no) via plain `:=`. Passing
  # ENABLE_DAGMC=yes on the make command line silently blocks that
  # canonicalization -- confirmed directly: build_embree/build_doubledown
  # never get their ON/OFF-gated stub definitions (Makefile:308-314) in
  # that case, so `make` fails with "No rule to make target 'build_embree'"
  # despite ENABLE_DOUBLE_DOWN being off, not because it's mis-detected.
  # An environment variable doesn't have this problem: it's the lowest-
  # precedence source of a variable's value, so the makefile's own
  # reassignments apply normally, exactly like a native user's `export
  # ENABLE_DAGMC=yes; make` would.
  COMMAND ${CMAKE_COMMAND} -E env
          NEKRS_HOME=${CARDINAL_INSTALL_DIR}
          CONTRIB_INSTALL_DIR=${CARDINAL_INSTALL_DIR}
          HDF5_ROOT=${HDF5_ROOT}
          PETSC_DIR=${PETSC_DIR}
          LIBMESH_DIR=${LIBMESH_DIR}
          WASP_DIR=${WASP_DIR}
          ${_make_enable_args}
          --
          $(MAKE) -f Makefile.cardinal
  WORKING_DIRECTORY ${CARDINAL_STAGE_DIR}
  DEPENDS ${_cardinal_depends}
  COMMENT "Building Cardinal via its own Makefile in ${CARDINAL_STAGE_DIR}"
  USES_TERMINAL
  VERBATIM)
