# cmake -P script: patches MOAB's own installed CMake package config
# (MOABConfig.cmake) so its MOAB_LIBRARIES variable never carries leading/
# trailing whitespace once evaluated.
#
# Built the way Cardinal's own config/moab.mk builds it (all optional
# package libraries -- BLAS/LAPACK/NetCDF/METIS/etc. -- off), MOAB's own
# generated config sets:
#   set(MOAB_LIBRARIES "-L<prefix>/lib ${MOAB_LIBS} ${MOAB_PACKAGE_LIBS}")
# which, with MOAB_PACKAGE_LIBS empty, evaluates at configure time (in
# whatever project consumes this file) to "-L<prefix>/lib -lMOAB " with a
# literal trailing space -- the space between the two ${} references
# survives even though the second one expands to nothing. Note this means
# the raw *text* of that line has no leading/trailing whitespace itself
# (regexing the string literal directly, as an earlier version of this
# script did, finds nothing to fix) -- the whitespace only appears once
# MOAB_PACKAGE_LIBS is substituted in by the consumer.
#
# Both DAGMC's and double-down's own CMakeLists.txt do
# target_link_libraries(<tgt> ${MOAB_LIBRARIES}), which trips CMake policy
# CMP0004 ("Libraries linked may not have leading or trailing whitespace")
# -- a hard error on every CMake new enough to matter.
#
# This is a real, reproducible bug in MOAB's own CMake export, independent
# of how MOAB is built -- Cardinal's own Makefile-driven build (identical
# cmake -L invocation, config/moab.mk) hits it exactly the same way in this
# same container. Rather than patch the MOAB/DAGMC/double-down submodule
# sources (out of scope for this superbuild), patch the generated config
# file in the install tree in place: insert a string(STRIP ...) call right
# after MOAB's own set(MOAB_LIBRARIES "...") line (the same technique this
# same file already uses a few lines above, on MOAB_PACKAGE_INCLUDES_LIST),
# so the fix applies at evaluation time regardless of what the embedded
# ${MOAB_LIBS}/${MOAB_PACKAGE_LIBS} references expand to. Runs after MOAB
# installs and before DAGMC/double-down configure against it (see the
# moab_fixup custom target in CMakeLists.txt).
#
# Expected -D argument: MOAB_CONFIG (path to the installed MOABConfig.cmake).
cmake_minimum_required(VERSION 3.21)

if(NOT MOAB_CONFIG)
  message(FATAL_ERROR "CardinalFixupMoabConfig.cmake: MOAB_CONFIG is required")
endif()
if(NOT EXISTS "${MOAB_CONFIG}")
  message(FATAL_ERROR "CardinalFixupMoabConfig.cmake: '${MOAB_CONFIG}' does not exist")
endif()

file(READ "${MOAB_CONFIG}" _contents)

if(_contents MATCHES "string\\(STRIP \"\\\${MOAB_LIBRARIES}\" MOAB_LIBRARIES\\)")
  message(STATUS "Cardinal: ${MOAB_CONFIG} already patched; nothing to do")
  return()
endif()

string(REGEX MATCH "set\\(MOAB_LIBRARIES \"[^\"]*\"\\)" _match "${_contents}")
if(NOT _match)
  message(WARNING "Cardinal: could not find 'set(MOAB_LIBRARIES \"...\")' in ${MOAB_CONFIG} -- nothing patched")
  return()
endif()

string(REPLACE "${_match}" "${_match}\nstring(STRIP \"\${MOAB_LIBRARIES}\" MOAB_LIBRARIES)" _contents "${_contents}")
file(WRITE "${MOAB_CONFIG}" "${_contents}")
message(STATUS "Cardinal: patched ${MOAB_CONFIG} to strip whitespace from MOAB_LIBRARIES at evaluation time")
