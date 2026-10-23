# cardinal_add_nek5000_tools(<name>
#   LOCAL_PATH  <path under CMAKE_SOURCE_DIR a local checkout, if any, lives at>
#   SOURCE_DIR  <where the checkout must live in the build tree -- Nek5000's
#               tools build in place, so this doubles as the build dir>
#   INSTALL_DIR <bin directory the built tool executables should land in --
#               forwarded as maketools' own "bin_nek_tools">
#   GIT_URL     <clone URL, used only when no local checkout exists>
#   TOOLS       <maketools argument(s), e.g. "core", "all", or a
#               space-separated list of tool names -- see tools/maketools>
#   FC / CC     <compilers>
#   MAXNEL      <maketools' own MAXNEL -- max number of elements a tool like
#               genbox/genmap can be compiled to handle>
#   [DEPENDS ...])
#
# Resolved the same way as any of Cardinal's real git submodules
# (cardinal_add_submodule_dependency) -- mirror an in-source checkout if
# present, else clone -- but deliberately isn't one: unlike every other
# contrib/ dependency, there's no commit to pin (the user asked for
# whatever the latest Nek5000 is, not a fixed version tracked in
# .gitmodules), and Nek5000 has no CMake build of its own to configure, just
# tools/maketools (a bash script driving a handful of tiny per-tool
# Makefiles). So this reimplements just the two resolution tiers that
# actually apply here, rather than reusing that function outright:
#   - already checked out under LOCAL_PATH: mirrored into the build tree on
#     every build, exactly like a real submodule would be
#     (CardinalMirrorSubmodule.cmake already handles a plain, non-submodule
#     .git directory correctly -- see its own header comment).
#   - otherwise: a plain shallow clone of the current default-branch tip.
#     ExternalProject's own native GIT_REPOSITORY/GIT_TAG support (rather
#     than CardinalGitFetchBySha.cmake's harder-case handling, used for
#     Cardinal's real, pinned submodules) is fine here -- that script exists
#     to fetch a commit that might not be reachable from any branch tip;
#     here we deliberately always want whatever the current tip *is*.
#     UPDATE_DISCONNECTED so it's a true one-time clone -- there's no
#     pinned commit to re-sync to, and silently re-fetching a moving
#     "latest" on every later build would make the tools' exact version
#     depend on when you last happened to build, not a deliberate choice
#     (as with every other dependency here, delete the build tree to pick
#     up a newer Nek5000).
#
# maketools itself builds in place under SOURCE_DIR/tools (relative paths
# baked into the script -- ./maketools.inc, ../bin, ...), so BINARY_DIR is
# set there directly rather than the usual separate build tree, and its own
# per-tool Makefiles have at least one target (the final link) that's
# unconditionally re-run rather than properly no-op'd when unchanged
# (confirmed: e.g. tools/genbox/makefile's "genbox" target names a file
# that's never actually created at that path -- the recipe writes to
# $(prefix)/genbox instead -- so it can never be seen as up to date); rather
# than fight that upstream, BUILD_ALWAYS is just accepted as this
# dependency's normal cost, same as the MOOSE prereq scripts' own
# `--fast` step -- a handful of small Fortran/C tools relink in well under
# a second.
include_guard(GLOBAL)
include(ExternalProject)
include(CardinalDependency)

function(cardinal_add_nek5000_tools name)
  set(oneValueArgs LOCAL_PATH SOURCE_DIR INSTALL_DIR GIT_URL TOOLS FC CC MAXNEL)
  set(multiValueArgs DEPENDS)
  cmake_parse_arguments(DEP "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  cardinal_submodule_populated(${DEP_LOCAL_PATH} _populated MARKER_FILE tools/maketools)

  set(_prefix ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${name}-prefix)

  # bash -c, not a bare BUILD_COMMAND list: maketools takes its tool
  # selection as separate positional words (e.g. "core", or "genbox
  # genmap"), and DEP_TOOLS is a single, possibly multi-word string -- a
  # shell splits that itself exactly like a user typing `./maketools
  # <TOOLS>` by hand would get, with no CMake-side list-vs-string
  # bookkeeping to keep in sync with maketools' own argument parsing.
  set(_build_cmd bash -c
    "mkdir -p '${DEP_INSTALL_DIR}' && bash maketools ${DEP_TOOLS}")

  if(_populated)
    set(_in_source_dir ${CMAKE_SOURCE_DIR}/${DEP_LOCAL_PATH})
    message(STATUS "Cardinal: '${DEP_LOCAL_PATH}' found checked out in-source; mirroring into the build tree for ${name}")

    set(_sync_cmd ${CMAKE_COMMAND}
      -DIN_SOURCE_DIR=${_in_source_dir}
      -DDEST_DIR=${DEP_SOURCE_DIR}
      -P ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/CardinalMirrorSubmodule.cmake)
    ExternalProject_Add(${name}
      PREFIX                  ${_prefix}
      SOURCE_DIR              ${DEP_SOURCE_DIR}
      BINARY_DIR              ${DEP_SOURCE_DIR}/tools
      DOWNLOAD_COMMAND        ${_sync_cmd}
      UPDATE_COMMAND          ${_sync_cmd}
      CONFIGURE_COMMAND       ${CMAKE_COMMAND} -E true
      BUILD_COMMAND           ${CMAKE_COMMAND} -E env
                                FC=${DEP_FC} CC=${DEP_CC} MAXNEL=${DEP_MAXNEL}
                                bin_nek_tools=${DEP_INSTALL_DIR}
                                -- ${_build_cmd}
      BUILD_ALWAYS            TRUE
      INSTALL_COMMAND         ${CMAKE_COMMAND} -E true
      DEPENDS                 ${DEP_DEPENDS}
      STEP_TARGETS            download;build
      USES_TERMINAL_DOWNLOAD  TRUE
      USES_TERMINAL_BUILD     TRUE)
  else()
    message(STATUS "Cardinal: '${DEP_LOCAL_PATH}' not present in-source; ${name} will be cloned from ${DEP_GIT_URL} (latest) directly into the build tree")

    ExternalProject_Add(${name}
      PREFIX                  ${_prefix}
      SOURCE_DIR              ${DEP_SOURCE_DIR}
      BINARY_DIR              ${DEP_SOURCE_DIR}/tools
      GIT_REPOSITORY          ${DEP_GIT_URL}
      GIT_TAG                 master
      GIT_SHALLOW             TRUE
      GIT_PROGRESS            TRUE
      # "" (CMake's documented way to fetch no submodules), not the
      # default: Nek5000's own .gitmodules pulls in the whole separate
      # NekExamples repo (its "examples" submodule) -- irrelevant to
      # building tools/, and not something a tools-only build should need
      # to clone.
      GIT_SUBMODULES          ""
      UPDATE_DISCONNECTED     TRUE
      CONFIGURE_COMMAND       ${CMAKE_COMMAND} -E true
      BUILD_COMMAND           ${CMAKE_COMMAND} -E env
                                FC=${DEP_FC} CC=${DEP_CC} MAXNEL=${DEP_MAXNEL}
                                bin_nek_tools=${DEP_INSTALL_DIR}
                                -- ${_build_cmd}
      BUILD_ALWAYS            TRUE
      INSTALL_COMMAND         ${CMAKE_COMMAND} -E true
      DEPENDS                 ${DEP_DEPENDS}
      STEP_TARGETS            download;build
      USES_TERMINAL_DOWNLOAD  TRUE
      USES_TERMINAL_BUILD     TRUE)
  endif()
endfunction()
