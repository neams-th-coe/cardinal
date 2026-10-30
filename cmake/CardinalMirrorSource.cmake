# cmake -P script used as mirror_source's actual build-time command (see
# CMakeLists.txt). Mirrors Cardinal's source tree into the build
# directory via rsync, excluding submodules, this build's own files,
# native-Makefile build/install byproducts, and any other CMake build
# directory sitting next to this one. The exclude list is computed fresh
# on every invocation rather than fixed at configure time -- see
# doc/content/cmake_details.md ("mirror_source: what gets copied, and
# how") for the full rationale behind each exclude and why it has to be
# computed this way.
#
# Expected -D arguments: SOURCE_DIR, DEST_DIR, ENABLE_NEK (ON/OFF).
cmake_minimum_required(VERSION 3.21)

if(NOT SOURCE_DIR OR NOT DEST_DIR)
  message(FATAL_ERROR "CardinalMirrorSource.cmake: SOURCE_DIR and DEST_DIR are required")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/CardinalByproductExcludes.cmake)

set(_excludes
  --exclude=/contrib
  --exclude=/.git
  --exclude=/Makefile
  --exclude=/unit/Makefile
  --exclude=/build
  --exclude=/install
  --exclude=/CMakeLists.txt
  --exclude=/cmake
  ${CARDINAL_BYPRODUCT_EXCLUDES})

if(ENABLE_NEK)
  list(APPEND _excludes --exclude=/test/tests/nek_ci)
endif()

# Also exclude any other CMake build directory next to this one
# (including this one, by the time this runs -- see cmake_details.md).
file(GLOB _top_level_entries LIST_DIRECTORIES true ${SOURCE_DIR}/*)
foreach(_entry ${_top_level_entries})
  if(IS_DIRECTORY ${_entry} AND EXISTS ${_entry}/CMakeCache.txt)
    get_filename_component(_name ${_entry} NAME)
    list(APPEND _excludes --exclude=/${_name})
  endif()
endforeach()

file(MAKE_DIRECTORY ${DEST_DIR})

execute_process(
  COMMAND rsync -rlpgo --update ${_excludes} ${SOURCE_DIR}/ ${DEST_DIR}/
  RESULT_VARIABLE _rc)
if(_rc)
  message(FATAL_ERROR "Cardinal: failed to mirror '${SOURCE_DIR}' into '${DEST_DIR}'")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/CardinalDetectMirrorDivergence.cmake)
cardinal_detect_mirror_divergence(${SOURCE_DIR} ${DEST_DIR} ${_excludes})
