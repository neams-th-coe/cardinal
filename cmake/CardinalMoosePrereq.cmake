include_guard(GLOBAL)
include(ExternalProject)

# cardinal_add_moose_prereq(<name>
#   SCRIPT   <absolute path to contrib/moose/scripts/update_and_rebuild_*.sh>
#   SOURCE_DIR <the dependency's natural in-tree location, e.g.
#              <moose>/petsc -- informational/tracking only; the script
#              itself decides where it operates from its own location>
#   ENV      <VAR=value ...>       # exported for both CONFIGURE and BUILD
#   [SCRIPT_ARGS <extra args, CONFIGURE (full run) only>]
#   [DEPENDS ...])
#
# These scripts couple configure+build into one call in their default mode,
# but ExternalProject tracks configure/build as independent steps with
# independent staleness -- see cmake/PLAN.md, "Phase 3", for the full
# reasoning. Deliberately split:
#   - CONFIGURE_COMMAND = the full non-`--fast` script: the *slow* one. It
#     wipes and reconfigures+rebuilds from scratch, including fetching its
#     own MOOSE-nested submodule (petsc/libmesh/wasp) if not already
#     populated -- this is what transparently handles a MOOSE checkout that
#     doesn't have (all of) its own submodules initialized.
#   - BUILD_COMMAND = the `--fast` invocation (build+install only, no
#     configure, no fetch), with BUILD_ALWAYS so it reruns every
#     `cmake --build` -- cheap when nothing changed, but picks up someone
#     hand-editing the dependency's source in place without forcing a full
#     reconfigure.
#   - INSTALL_COMMAND = no-op: both of the above already install
#     internally.
#   - DOWNLOAD_COMMAND = no-op: the script handles its own submodule fetch.
function(cardinal_add_moose_prereq name)
  set(oneValueArgs SCRIPT SOURCE_DIR)
  set(multiValueArgs ENV SCRIPT_ARGS DEPENDS)
  cmake_parse_arguments(DEP "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(_prefix ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${name}-prefix)

  ExternalProject_Add(${name}
    PREFIX              ${_prefix}
    SOURCE_DIR          ${DEP_SOURCE_DIR}
    BINARY_DIR          ${DEP_SOURCE_DIR}
    DOWNLOAD_COMMAND    ${CMAKE_COMMAND} -E true
    CONFIGURE_COMMAND   ${CMAKE_COMMAND} -E env ${DEP_ENV} --
                        bash ${DEP_SCRIPT} ${DEP_SCRIPT_ARGS}
    BUILD_COMMAND       ${CMAKE_COMMAND} -E env ${DEP_ENV} --
                        bash ${DEP_SCRIPT} --fast
    BUILD_ALWAYS        TRUE
    INSTALL_COMMAND     ${CMAKE_COMMAND} -E true
    DEPENDS             ${DEP_DEPENDS}
    STEP_TARGETS        configure;build
    USES_TERMINAL_CONFIGURE TRUE
    USES_TERMINAL_BUILD     TRUE)
endfunction()
