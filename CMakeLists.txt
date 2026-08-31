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

if(ENABLE_DOUBLE_DOWN AND NOT ENABLE_DAGMC)
  message(FATAL_ERROR "ENABLE_DOUBLE_DOWN requires ENABLE_DAGMC")
endif()

# DAGMC/MOAB/Embree/double-down sub-builds are not wired up yet (Phase 2).
if(ENABLE_DAGMC)
  message(FATAL_ERROR "ENABLE_DAGMC is not implemented yet in the CMake build (Phase 2); configure with -DENABLE_DAGMC=OFF for now.")
endif()

set(CARDINAL_BUILD_TYPE "Release" CACHE STRING "Build type forwarded to dependency CMake sub-builds (Release or Debug)")
set_property(CACHE CARDINAL_BUILD_TYPE PROPERTY STRINGS Release Debug)

# -----------------------------------------------------------------------------
# Layout
#
# Everything Make-related (mirrored app source, contrib/<dep>, build/<dep>,
# install/) lives under one subdirectory of the CMake build tree rather than
# at its root, so CMake's own generated build files (Makefile,
# CMakeCache.txt, ...) don't collide with Cardinal's own hand-written
# top-level Makefile of the same name. See cmake/PLAN.md.
# -----------------------------------------------------------------------------
set(CARDINAL_STAGE_DIR   ${CMAKE_BINARY_DIR}/cardinal)
set(CARDINAL_INSTALL_DIR ${CARDINAL_STAGE_DIR}/install)

file(MAKE_DIRECTORY ${CARDINAL_STAGE_DIR})

# Mirror everything except the large, separately-managed submodules and
# CMake/Make build byproducts. Reruns every build (add_custom_target has no
# tracked OUTPUT, so it's always considered out of date) -- the same role as
# the manual `rsync -a` in SYNC_tmp/SYNC_nvme, but automatic.
add_custom_target(mirror_source ALL
  COMMAND rsync -a --delete
          --exclude=/contrib/moose
          --exclude=/contrib/nekRS
          --exclude=/contrib/openmc
          --exclude=/contrib/nuclear_data
          --exclude=/build
          --exclude=/install
          --exclude=/.git
          ${CMAKE_SOURCE_DIR}/ ${CARDINAL_STAGE_DIR}/
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
  # an explicit choice. See cmake/PLAN.md, Phase 3, "Parallelism".
  set(CARDINAL_MOOSE_BUILD_PARALLELISM "" CACHE STRING "Parallel jobs (-j) for building PETSc/libMesh/WASP from source. Required when building from source.")
  if(NOT CARDINAL_MOOSE_BUILD_PARALLELISM)
    message(FATAL_ERROR
      "Cardinal: building PETSc/libMesh/WASP from source requires "
      "-DCARDINAL_MOOSE_BUILD_PARALLELISM=<N> -- these scripts can't share "
      "cmake --build's own -j. See cmake/PLAN.md, Phase 3, Parallelism.")
  endif()

  set(PETSC_ARCH "arch-moose" CACHE STRING "PETSc build arch (matches update_and_rebuild_petsc.sh/Makefile's own default)")

  if(CARDINAL_BUILD_TYPE STREQUAL "Debug")
    set(_moose_method dbg)
  else()
    set(_moose_method opt)
  endif()

  # Matches what each script installs to by default (Makefile:73-75) --
  # PETSc installs in place at $PETSC_DIR/$PETSC_ARCH (no PETSC_PREFIX set).
  set(_moose_dir     ${CARDINAL_STAGE_DIR}/contrib/moose)
  set(_moose_scripts ${_moose_dir}/scripts)
  set(PETSC_DIR   ${_moose_dir}/petsc)
  set(LIBMESH_DIR ${_moose_dir}/libmesh/installed)
  set(WASP_DIR    ${_moose_dir}/framework/contrib/wasp/install)
  set(HDF5_ROOT   ${PETSC_DIR}/${PETSC_ARCH})

  cardinal_add_moose_prereq(petsc
    SCRIPT     ${_moose_scripts}/update_and_rebuild_petsc.sh
    SOURCE_DIR ${_moose_dir}/petsc
    ENV        CC=${LIBMESH_CC} CXX=${LIBMESH_CXX} FC=${LIBMESH_FC}
               PETSC_ARCH=${PETSC_ARCH}
               MOOSE_JOBS=${CARDINAL_MOOSE_BUILD_PARALLELISM}
    DEPENDS    moose)

  cardinal_add_moose_prereq(libmesh
    SCRIPT     ${_moose_scripts}/update_and_rebuild_libmesh.sh
    SOURCE_DIR ${_moose_dir}/libmesh
    ENV        CC=${LIBMESH_CC} CXX=${LIBMESH_CXX} FC=${LIBMESH_FC}
               PETSC_DIR=${PETSC_DIR} PETSC_ARCH=${PETSC_ARCH}
               METHOD=${_moose_method}
               MOOSE_JOBS=${CARDINAL_MOOSE_BUILD_PARALLELISM}
    DEPENDS    moose petsc)

  cardinal_add_moose_prereq(wasp
    SCRIPT     ${_moose_scripts}/update_and_rebuild_wasp.sh
    SOURCE_DIR ${_moose_dir}/framework/contrib/wasp
    ENV        CC=${LIBMESH_CC} CXX=${LIBMESH_CXX} FC=${LIBMESH_FC}
               MOOSE_JOBS=${CARDINAL_MOOSE_BUILD_PARALLELISM}
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
    set(HDF5_ROOT ${PETSC_DIR})
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
  cardinal_add_submodule_dependency(nekrs
    SUBMODULE_PATH contrib/nekRS
    SOURCE_DIR      ${CARDINAL_STAGE_DIR}/contrib/nekRS
    BINARY_DIR      ${CARDINAL_STAGE_DIR}/build/nekrs
    INSTALL_DIR     ${CARDINAL_INSTALL_DIR}
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=${CARDINAL_BUILD_TYPE}
      -DCMAKE_C_COMPILER=${LIBMESH_CC}
      -DCMAKE_CXX_COMPILER=${LIBMESH_CXX}
      -DCMAKE_Fortran_COMPILER=${LIBMESH_FC}
      -DCMAKE_INSTALL_PREFIX=${CARDINAL_INSTALL_DIR}
      -DCMAKE_INSTALL_MESSAGE=LAZY
      "-DNEK5000_PPLIST=PARRSB DPROCMAP"
      -DOCCA_CXX=${LIBMESH_CC}
      "-DOCCA_CXXFLAGS=-O2 -ftree-vectorize -funroll-loops -march=native -mtune=native"
      -DENABLE_CUDA=0
      -DENABLE_OPENCL=0
      -DENABLE_HIP=0
      -DENABLE_AMGX=0)
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
      -DCMAKE_BUILD_TYPE=${CARDINAL_BUILD_TYPE}
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
  list(APPEND _cardinal_depends nekrs)
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
          $(MAKE) ${_make_enable_args}
  WORKING_DIRECTORY ${CARDINAL_STAGE_DIR}
  DEPENDS ${_cardinal_depends}
  COMMENT "Building Cardinal via its own Makefile in ${CARDINAL_STAGE_DIR}"
  USES_TERMINAL
  VERBATIM)
