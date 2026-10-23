# Two independent container checks, each with its own FATAL/warning option:
# ignoring a stale MOOSE version pin is plausibly fine (it's exactly what
# MOOSE's own premake.py itself only ever warns about, never fails on), but
# a build tree currently disagreeing with the container it was configured
# under (below) means the compiler paths and other environment-derived
# settings it's about to build with likely belong to a *different*
# container than the one about to run -- a much easier way to silently
# corrupt a build, and unlikely to ever be intentional.
#   * cardinal-check-container (cmake/CardinalCheckContainer.cmake, below):
#     the running container vs. the one this build tree was configured
#     under.
#   * moose-check-container (cmake/CardinalCheckMooseVersion.cmake, defined
#     alongside the "moose" target in CMakeLists.txt): the running
#     container vs. what contrib/moose's own versioner.yaml says it should
#     be -- MOOSE's own native build does this in scripts/premake.py (the
#     "WARNING: Container ... is currently at version ..." message); that
#     check is silently disabled entirely in this build (see that file), so
#     this gives it real teeth here instead.
option(CONTAINER_DRIFT_CHECK_FATAL
  "FATAL_ERROR (rather than just a warning) when the running apptainer \
container doesn't match the one this build tree was configured under" ON)
option(CONTAINER_VERSION_CHECK_FATAL
  "FATAL_ERROR (rather than just a warning) when the running apptainer \
container doesn't match what MOOSE's own versioner.py says the pinned \
contrib/moose commit requires" ON)

# Snapshot the container identity present right now, at *configure* time --
# compared again at *build* time (cardinal-check-container, below) against
# whatever's actually running then, to catch the tree being built under a
# different container than it was configured under. CACHE INTERNAL: pure
# bookkeeping, not a user-facing setting -- doesn't show up in ccmake, and
# (unlike a plain `set`) persists across a build tree's later reconfigures
# that don't re-list it, so it keeps reflecting whatever `cmake`
# invocation *first* established the rest of this cache's still-standing
# container-derived settings, not just the most recent one -- see
# CardinalCheckContainer.cmake for why that's the comparison that matters.
if(NOT DEFINED CARDINAL_CONFIGURE_CONTAINER_NAME)
  set(CARDINAL_CONFIGURE_CONTAINER_NAME "$ENV{MOOSE_APPTAINER_GENERATOR_NAME}"
    CACHE INTERNAL "Container name recorded at configure time")
  set(CARDINAL_CONFIGURE_CONTAINER_VERSION "$ENV{MOOSE_APPTAINER_GENERATOR_VERSION}"
    CACHE INTERNAL "Container version recorded at configure time")
endif()

# Checks the currently-running container against the one this tree was
# configured under -- see cmake/CardinalCheckContainer.cmake for the full
# story. Deliberately has no DEPENDS of its own (unlike moose-check-container
# above, this doesn't need contrib/moose to exist) and is instead added as a
# DEPENDS of every other target that has none of its own -- mirror_source,
# moose-add, nuclear_data, nek_ci -- so it's unconditionally the first thing
# any build does, before anything else (a source mirror, a submodule fetch,
# ...) gets a chance to run under, and possibly be silently corrupted by, the
# wrong container.
add_custom_target(cardinal-check-container
  COMMAND ${CMAKE_COMMAND} -DCONTAINER_DRIFT_CHECK_FATAL=${CONTAINER_DRIFT_CHECK_FATAL}
          -DCONFIGURE_CONTAINER_NAME=${CARDINAL_CONFIGURE_CONTAINER_NAME}
          -DCONFIGURE_CONTAINER_VERSION=${CARDINAL_CONFIGURE_CONTAINER_VERSION}
          -P ${CMAKE_SOURCE_DIR}/cmake/CardinalCheckContainer.cmake
  COMMENT "Cardinal: checking the running container against configure time"
  VERBATIM)
