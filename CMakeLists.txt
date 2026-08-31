cmake_minimum_required(VERSION 3.20)
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
# Pre-built dependency locations (container-provided; Makefile:1-38, and the
# WASP_DIR/LIBMESH_DIR/PETSC_DIR/... env vars the build container sets).
# Phase 1 only *locates* these -- it never builds them, and it never touches
# MOOSE's own build system.
# -----------------------------------------------------------------------------
set(WASP_DIR    "$ENV{WASP_DIR}"    CACHE PATH "Top-level WASP install")
set(LIBMESH_DIR "$ENV{LIBMESH_DIR}" CACHE PATH "Top-level libMesh install")
set(PETSC_DIR   "$ENV{PETSC_DIR}"   CACHE PATH "Top-level PETSc install")
set(HDF5_ROOT   "$ENV{HDF5_ROOT}"   CACHE PATH "Top-level HDF5 install (default: PETSC_DIR)")

foreach(_var WASP_DIR LIBMESH_DIR PETSC_DIR)
  if(NOT ${_var})
    message(FATAL_ERROR
      "${_var} is not set. Source the build container's environment "
      "(it sets WASP_DIR/LIBMESH_DIR/PETSC_DIR), or configure with "
      "-D${_var}=<path>.")
  endif()
endforeach()

if(NOT HDF5_ROOT)
  set(HDF5_ROOT ${PETSC_DIR})
endif()

if(NOT EXISTS ${LIBMESH_DIR}/bin/libmesh-config)
  message(FATAL_ERROR "Could not find ${LIBMESH_DIR}/bin/libmesh-config -- is LIBMESH_DIR (${LIBMESH_DIR}) correct?")
endif()

# Discover the exact compiler triple libMesh (and therefore MOOSE/Cardinal)
# was built with. The Makefile normally derives this deep inside
# framework/build.mk, which only runs once `make` starts -- too late for our
# ExternalProject configure steps, which need it up front. See
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

  cardinal_add_submodule_dependency(openmc
    SUBMODULE_PATH contrib/openmc
    SOURCE_DIR      ${CARDINAL_STAGE_DIR}/contrib/openmc
    BINARY_DIR      ${CARDINAL_STAGE_DIR}/build/openmc
    INSTALL_DIR     ${CARDINAL_INSTALL_DIR}
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
