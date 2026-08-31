include_guard(GLOBAL)
include(ExternalProject)

# cardinal_gitmodule_url(<relative submodule path> <out var>)
#
# Looks up the URL recorded for a submodule in .gitmodules. Section names in
# Cardinal's .gitmodules are the submodule path itself (e.g.
# `[submodule "contrib/nekRS"]`), so the path can be used directly as the
# config key's subsection.
function(cardinal_gitmodule_url path out_var)
  execute_process(
    COMMAND git config -f ${CMAKE_SOURCE_DIR}/.gitmodules --get submodule.${path}.url
    OUTPUT_VARIABLE _url
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _rc
    ERROR_QUIET)
  if(_rc EQUAL 0)
    set(${out_var} ${_url} PARENT_SCOPE)
  else()
    set(${out_var} "" PARENT_SCOPE)
  endif()
endfunction()

# cardinal_gitmodule_commit(<relative submodule path> <out var>)
#
# Looks up the commit Cardinal's superproject git index pins a submodule to
# (the gitlink entry), so ExternalProject's GIT_TAG stays derived from
# .gitmodules/the git index rather than being hand-copied into CMake.
function(cardinal_gitmodule_commit path out_var)
  execute_process(
    COMMAND git -C ${CMAKE_SOURCE_DIR} ls-tree HEAD -- ${path}
    OUTPUT_VARIABLE _line
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _rc
    ERROR_QUIET)
  if(_rc EQUAL 0 AND _line MATCHES "^[0-9]+ commit ([0-9a-f]+)")
    set(${out_var} ${CMAKE_MATCH_1} PARENT_SCOPE)
  else()
    set(${out_var} "" PARENT_SCOPE)
  endif()
endfunction()

# cardinal_submodule_populated(<relative submodule path> <out var>)
#
# True if the submodule has actually been checked out in the source tree
# (as opposed to being an empty/absent placeholder for an uninitialized
# submodule). Keyed off CMakeLists.txt existing, the same file each
# dependency's own config/*.mk rule already keys its build off of.
function(cardinal_submodule_populated path out_var)
  if(EXISTS ${CMAKE_SOURCE_DIR}/${path}/CMakeLists.txt)
    set(${out_var} TRUE PARENT_SCOPE)
  else()
    set(${out_var} FALSE PARENT_SCOPE)
  endif()
endfunction()

# cardinal_add_submodule_dependency(<name>
#   SUBMODULE_PATH <path to the submodule, relative to CMAKE_SOURCE_DIR>
#   SOURCE_DIR     <where the dependency's source must live in the build tree>
#   BINARY_DIR     <where the dependency's own CMake build happens>
#   INSTALL_DIR    <shared install prefix>
#   CMAKE_ARGS     <args forwarded to the dependency's own CMake configure>
#   [DEPENDS ...])
#
# Three-tier resolution (see cmake/PLAN.md):
#   1. (handled by the caller -- this function only covers submodules
#      Cardinal itself builds; pre-built installs are just located)
#   2. Submodule already checked out in-source: mirror it into SOURCE_DIR,
#      resynced on every build, via ExternalProject's DOWNLOAD/UPDATE steps.
#   3. Otherwise: clone straight into SOURCE_DIR from .gitmodules'
#      URL/pinned commit; the source tree is never touched.
#
# In both cases the actual CMake configure/build/install of the dependency
# happens entirely under the CMake build tree.
function(cardinal_add_submodule_dependency name)
  set(oneValueArgs SUBMODULE_PATH SOURCE_DIR BINARY_DIR INSTALL_DIR)
  set(multiValueArgs CMAKE_ARGS DEPENDS)
  cmake_parse_arguments(DEP "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  cardinal_submodule_populated(${DEP_SUBMODULE_PATH} _populated)

  if(_populated)
    set(_in_source_dir ${CMAKE_SOURCE_DIR}/${DEP_SUBMODULE_PATH})
    message(STATUS "Cardinal: '${DEP_SUBMODULE_PATH}' found checked out in-source; mirroring into the build tree for ${name}")

    set(_sync_cmd ${CMAKE_COMMAND} -E copy_directory_if_newer ${_in_source_dir} ${DEP_SOURCE_DIR})
    ExternalProject_Add(${name}
      SOURCE_DIR             ${DEP_SOURCE_DIR}
      BINARY_DIR              ${DEP_BINARY_DIR}
      DOWNLOAD_COMMAND        ${_sync_cmd}
      UPDATE_COMMAND          ${_sync_cmd}
      CMAKE_ARGS              ${DEP_CMAKE_ARGS}
      INSTALL_DIR             ${DEP_INSTALL_DIR}
      DEPENDS                 ${DEP_DEPENDS}
      STEP_TARGETS            download;configure;build;install
      USES_TERMINAL_DOWNLOAD  TRUE
      USES_TERMINAL_BUILD     TRUE
      USES_TERMINAL_INSTALL   TRUE)
  else()
    cardinal_gitmodule_url(${DEP_SUBMODULE_PATH} _url)
    cardinal_gitmodule_commit(${DEP_SUBMODULE_PATH} _commit)
    if(NOT _url OR NOT _commit)
      message(FATAL_ERROR
        "Cardinal: could not determine the git URL/pinned commit for "
        "submodule '${DEP_SUBMODULE_PATH}' from .gitmodules / the git "
        "index -- is this a git checkout of Cardinal?")
    endif()
    message(STATUS "Cardinal: '${DEP_SUBMODULE_PATH}' not present in-source; ${name} will be cloned from ${_url} @ ${_commit} directly into the build tree")

    ExternalProject_Add(${name}
      SOURCE_DIR             ${DEP_SOURCE_DIR}
      BINARY_DIR              ${DEP_BINARY_DIR}
      GIT_REPOSITORY          ${_url}
      GIT_TAG                 ${_commit}
      GIT_SHALLOW             FALSE
      CMAKE_ARGS               ${DEP_CMAKE_ARGS}
      INSTALL_DIR             ${DEP_INSTALL_DIR}
      DEPENDS                 ${DEP_DEPENDS}
      STEP_TARGETS            download;configure;build;install
      USES_TERMINAL_DOWNLOAD  TRUE
      USES_TERMINAL_BUILD     TRUE
      USES_TERMINAL_INSTALL   TRUE)
  endif()
endfunction()
