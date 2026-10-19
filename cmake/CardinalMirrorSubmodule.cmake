# cmake -P script used as the DOWNLOAD_COMMAND/UPDATE_COMMAND for the tier-2
# ("already checked out in-source") path in cardinal_add_submodule_dependency.
#
# Mirrors an in-source submodule's working tree into the build tree via
# rsync (mtime-based, --update), and separately repairs the destination's
# own git metadata so a git command later run from *inside* the mirrored
# copy (e.g. MOOSE's update_and_rebuild_*.sh scripts, which run `git
# submodule update --init` on their own nested libmesh/petsc/wasp
# submodules) operates on the mirror itself, not the original source.
#
# A submodule's .git is a gitlink file pointing at the real git directory
# nested under the superproject's own .git/modules/<path>. Copying that
# file verbatim into the build tree is unsafe either way: the path it
# points at doesn't exist in the mirror (mirror_source excludes /.git
# from the whole-repo mirror), breaking anything that needs git; and if
# instead repointed at the *original* gitdir's absolute path, that
# gitdir's own config sets core.worktree back to the original checkout,
# which git honors regardless of where the gitlink pointing at it lives
# -- so git commands would silently operate on the original source tree,
# not the mirror. Fixed by copying the real git directory's contents into
# <dest>/.git (a real directory, not a gitlink) and stripping any
# core.worktree override from its config, making the copy a genuinely
# self-contained repository whose worktree is itself.
#
# Expected -D arguments: IN_SOURCE_DIR, DEST_DIR.
cmake_minimum_required(VERSION 3.21)

if(NOT IN_SOURCE_DIR OR NOT DEST_DIR)
  message(FATAL_ERROR "CardinalMirrorSubmodule.cmake: IN_SOURCE_DIR and DEST_DIR are required")
endif()

file(MAKE_DIRECTORY "${DEST_DIR}")

# CARDINAL_BYPRODUCT_EXCLUDES (shared with CardinalMirrorSource.cmake --
# see that file) keeps a prior native, in-place build of this same
# checkout from getting mirrored on top of a fresh one.
include(${CMAKE_CURRENT_LIST_DIR}/CardinalByproductExcludes.cmake)

execute_process(
  COMMAND rsync -rlpgo --update --exclude=.git ${CARDINAL_BYPRODUCT_EXCLUDES} "${IN_SOURCE_DIR}/" "${DEST_DIR}/"
  RESULT_VARIABLE _rc)
if(_rc)
  message(FATAL_ERROR "Cardinal: failed to mirror '${IN_SOURCE_DIR}' into '${DEST_DIR}'")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/CardinalDetectMirrorDivergence.cmake)
cardinal_detect_mirror_divergence("${IN_SOURCE_DIR}" "${DEST_DIR}" --exclude=.git ${CARDINAL_BYPRODUCT_EXCLUDES})

if(NOT EXISTS "${IN_SOURCE_DIR}/.git")
  return()
endif()

if(IS_DIRECTORY "${IN_SOURCE_DIR}/.git")
  # Not itself a submodule (a real .git directory already) -- nothing to
  # dereference/fix up, just mirror it like any other directory.
  set(_real_gitdir "${IN_SOURCE_DIR}/.git")
else()
  execute_process(
    COMMAND git rev-parse --absolute-git-dir
    WORKING_DIRECTORY "${IN_SOURCE_DIR}"
    OUTPUT_VARIABLE _real_gitdir
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _rc)
  if(_rc)
    message(FATAL_ERROR "Cardinal: failed to resolve the real git directory for '${IN_SOURCE_DIR}'")
  endif()
endif()

execute_process(
  COMMAND rsync -rlpgo --update "${_real_gitdir}/" "${DEST_DIR}/.git/"
  RESULT_VARIABLE _rc)
if(_rc)
  message(FATAL_ERROR "Cardinal: failed to mirror git directory '${_real_gitdir}' into '${DEST_DIR}/.git'")
endif()

# Strip any core.worktree override -- see the file-level comment above.
# `--file <path>` targets the config file directly, rather than going
# through git's normal repo/worktree discovery (which is exactly what's
# broken here, since core.worktree still points at the original checkout
# at this point). `git config --unset` exits 5 (not 0) when the key was
# never set, which isn't an error here -- e.g. IN_SOURCE_DIR wasn't
# itself a submodule. That exit code is checked explicitly, rather than
# swallowed outright, so a genuine transient failure isn't
# indistinguishable from the harmless "wasn't set" case; one retry is
# cheap insurance against such a failure, and a warning (not a hard
# error) surfaces if the config still isn't clean afterward.
execute_process(
  COMMAND git config --file "${DEST_DIR}/.git/config" --unset core.worktree
  RESULT_VARIABLE _unset_rc)
if(NOT _unset_rc EQUAL 0 AND NOT _unset_rc EQUAL 5)
  execute_process(
    COMMAND git config --file "${DEST_DIR}/.git/config" --unset core.worktree
    RESULT_VARIABLE _unset_rc)
endif()
file(STRINGS "${DEST_DIR}/.git/config" _worktree_line REGEX "^[ \t]*worktree[ \t]*=")
if(_worktree_line)
  message(WARNING
    "Cardinal: failed to strip core.worktree from '${DEST_DIR}/.git/config' "
    "(git config exit code ${_unset_rc}) -- git commands run from inside "
    "'${DEST_DIR}' may fail or silently operate on '${IN_SOURCE_DIR}' instead.")
endif()
