cmake_minimum_required(VERSION 3.21)

# Checks that the running apptainer container matches what's pinned for the
# checked-out contrib/moose commit -- MOOSE's own native build does this in
# scripts/premake.py (the "WARNING: Container ... is currently at version
# ..." message), via its Versioner class. That specific mechanism can't be
# reused directly here: Versioner.get_packages() unconditionally resolves
# an unrelated "app" package via a git call that requires the *calling
# app's* (Cardinal's) working directory to be a git checkout, which the
# mirrored build tree deliberately isn't (see mirror_source in
# CMakeLists.txt) -- so that call always fails, and with it every check
# premake.py was going to run, this one included.
#
# Reimplemented directly instead of routing around that: read straight from
# MOOSE's own scripts/versioner.yaml. For the package named by
# MOOSE_APPTAINER_GENERATOR_LIBRARY, everything premake.py's own
# ApptainerPackage computation reduces to (confirmed by reading
# versioner.py directly) is that package's `version:` field, with
# `_<build_number>` appended if `build_number:` is also present --
# moose-dev, the only apptainer package Cardinal's own build ever runs
# inside, has never had a build_number. No hashing, no git history walk
# needed for this specific value.
#
# Invoked as its own standalone `moose-check-container` custom target
# (CMakeLists.txt), DEPENDS moose-add -- needs contrib/moose's own source to
# read versioner.yaml, but nothing built. See CardinalCheckContainer.cmake
# for the *other*, unrelated container check (configure-time vs build-time
# drift) that doesn't need moose at all.
if(NOT DEFINED MOOSE_DIR OR MOOSE_DIR STREQUAL "")
  message(FATAL_ERROR "CardinalCheckMooseVersion.cmake: MOOSE_DIR is required")
endif()
if(NOT DEFINED CONTAINER_VERSION_CHECK_FATAL)
  message(FATAL_ERROR "CardinalCheckMooseVersion.cmake: CONTAINER_VERSION_CHECK_FATAL is required")
endif()

set(_library "$ENV{MOOSE_APPTAINER_GENERATOR_LIBRARY}")
set(_current_name "$ENV{MOOSE_APPTAINER_GENERATOR_NAME}")
set(_current_version "$ENV{MOOSE_APPTAINER_GENERATOR_VERSION}")

# Nothing to check outside an apptainer container built by MOOSE's own
# generator -- matches premake.py's own `if self.apptainer_env:` guard.
if(_library STREQUAL "")
  return()
endif()

set(_versioner_yaml ${MOOSE_DIR}/scripts/versioner.yaml)
if(NOT EXISTS ${_versioner_yaml})
  return()  # can't determine the required version -- fail open, matching
            # premake.py's own tolerance for an environment it can't check
endif()

# versioner.yaml's packages: block is a flat list of 2-space-indented
# "<name>:" keys, each with 4-space-indented "version:"/"build_number:"
# scalars among its own fields -- find the block for our package, then
# pull those two fields out of it.
file(STRINGS ${_versioner_yaml} _lines)
set(_in_package FALSE)
set(_required_version "")
set(_required_build_number "")
foreach(_line IN LISTS _lines)
  if(_line MATCHES "^  ${_library}:[ \t]*$")
    set(_in_package TRUE)
  elseif(_in_package AND _line MATCHES "^  [A-Za-z0-9_-]+:")
    break()  # the next package's block started
  elseif(_in_package AND _line MATCHES "^    version:[ \t]*(.+)$")
    set(_required_version "${CMAKE_MATCH_1}")
  elseif(_in_package AND _line MATCHES "^    build_number:[ \t]*(.+)$")
    set(_required_build_number "${CMAKE_MATCH_1}")
  endif()
endforeach()

if(_required_version STREQUAL "")
  return()  # package not found, or has no explicit version (hash-based --
            # doesn't apply to moose-dev today); fail open
endif()

set(_required "${_required_version}")
if(NOT _required_build_number STREQUAL "")
  set(_required "${_required}_${_required_build_number}")
endif()

if(_required STREQUAL _current_version)
  return()
endif()

# Matches premake.py's own ApptainerVersionMismatch message, minus the
# INL-HPC "loaded module" branch (Cardinal's own docs only ever reference
# the oras:// URL directly, never a module load, for this container).
set(_name_base "${_library}")
if(NOT _name_base MATCHES "^moose-")
  set(_name_base "moose-${_name_base}")
endif()
if(_name_base STREQUAL "moose-dev")
  set(_harbor "harbor.hpc.inl.gov")
else()
  set(_harbor "mooseharbor.hpc.inl.gov")
endif()

string(CONCAT _message
  "Container ${_current_name} is currently at version ${_current_version} and the "
  "required version is ${_required}. Before updating the container, make "
  "sure that your version of MOOSE is up to date. You can obtain the "
  "correct container at oras://${_harbor}/${_name_base}/${_current_name}:${_required}.")

if(CONTAINER_VERSION_CHECK_FATAL)
  message(FATAL_ERROR
    "Cardinal: ${_message} Reconfigure with -DCONTAINER_VERSION_CHECK_FATAL=OFF "
    "to downgrade this to a warning instead.")
else()
  message(WARNING "Cardinal: ${_message}")
endif()
