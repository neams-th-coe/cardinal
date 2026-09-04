cmake_minimum_required(VERSION 3.21)

# Detects a container swapped out from under an already-configured build
# tree -- e.g. a persistent apptainer sandbox rebuilt in place, or a build
# tree reused under a different container/.sif entirely. Compiler paths and
# other environment-derived settings (HDF5_ROOT, LIBMESH_DIR, PETSC_DIR,
# ...) are all resolved once, at `cmake` configure time, from whichever
# container happened to be running then -- and, like the recorded container
# identity this compares against, are cached, sticky CMake variables that a
# later reconfigure of the same build tree won't refresh. A build tree
# reused under a swapped sandbox/.sif is exactly the case that produced
# OpenMC's own confusing "non-existent path .../vtk-9.7" configure failure
# during testing -- not a version-pin mismatch (see
# CardinalCheckMooseVersion.cmake for that, separate, check), a stale
# CMakeCache pointing at paths that belonged to a different container.
#
# Invoked as its own standalone `cardinal-check-container` custom target
# (CMakeLists.txt) -- a plain add_custom_target with no OUTPUT/BYPRODUCTS is
# always considered out of date, so this genuinely re-runs on every build,
# not just at configure time. Deliberately doesn't need contrib/moose (or
# anything else) to exist first, so every other target that has no
# dependency of its own DEPENDS directly on this one -- making it
# unconditionally the first thing any build does.
if(NOT DEFINED CONTAINER_DRIFT_CHECK_FATAL)
  message(FATAL_ERROR "CardinalCheckContainer.cmake: CONTAINER_DRIFT_CHECK_FATAL is required")
endif()
# CONFIGURE_CONTAINER_NAME/VERSION are optional (empty string if not given,
# matching how they'd read if this script were ever invoked standalone,
# e.g. for manual testing).
if(NOT DEFINED CONFIGURE_CONTAINER_NAME)
  set(CONFIGURE_CONTAINER_NAME "")
endif()
if(NOT DEFINED CONFIGURE_CONTAINER_VERSION)
  set(CONFIGURE_CONTAINER_VERSION "")
endif()

set(_current_name "$ENV{MOOSE_APPTAINER_GENERATOR_NAME}")
set(_current_version "$ENV{MOOSE_APPTAINER_GENERATOR_VERSION}")

# The only silent case is empty-to-empty (never used a container, either
# time) -- this fires just as much for a tree configured outside any
# container that's now being built inside one (or vice versa) as for two
# different containers, since either one is the same underlying drift.
if(_current_name STREQUAL CONFIGURE_CONTAINER_NAME AND
   _current_version STREQUAL CONFIGURE_CONTAINER_VERSION)
  return()
endif()

if(CONFIGURE_CONTAINER_NAME STREQUAL "")
  set(_configured_as "no container")
else()
  set(_configured_as "'${CONFIGURE_CONTAINER_NAME}' (version ${CONFIGURE_CONTAINER_VERSION})")
endif()
if(_current_name STREQUAL "")
  set(_building_as "no container")
else()
  set(_building_as "'${_current_name}' (version ${_current_version})")
endif()

string(CONCAT _drift_message
  "This build tree was configured under ${_configured_as} but is now "
  "building under ${_building_as}. Compiler paths and other environment-"
  "derived settings (HDF5_ROOT, LIBMESH_DIR, PETSC_DIR, ...) were "
  "captured at configure time -- and, like this recorded container "
  "identity itself, are cached, sticky CMake variables that a plain "
  "reconfigure won't refresh. Start a fresh build tree against the "
  "current container instead.")

if(CONTAINER_DRIFT_CHECK_FATAL)
  message(FATAL_ERROR
    "Cardinal: ${_drift_message} Reconfigure with -DCONTAINER_DRIFT_CHECK_FATAL=OFF "
    "to downgrade this to a warning instead.")
else()
  message(WARNING "Cardinal: ${_drift_message}")
endif()
