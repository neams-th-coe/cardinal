# cmake -P script used as the DOWNLOAD_COMMAND/UPDATE_COMMAND for the tier-2
# ("already checked out in-source") path in cardinal_add_submodule_dependency.
#
# Mirrors an in-source submodule's working tree into the build tree via
# rsync (mtime-based, --update, so only changed files are re-copied on each
# build -- see that function for the full rationale), and separately
# repairs the destination's own git metadata so any git command later run
# from *inside* the mirrored copy (e.g. MOOSE's update_and_rebuild_*.sh
# scripts run `git submodule update --init` on their own nested libmesh/
# petsc/wasp submodules) operates correctly against the mirrored copy
# itself, rather than failing or silently touching the original source.
#
# Why this needs its own handling rather than a plain rsync of the
# directory (.git included): a submodule's .git is a *gitlink* file
# (`gitdir: <relative path>`) pointing at the real git directory nested
# under the superproject's own .git/modules/<path> -- copying that file
# verbatim into the build tree either
#   (a) points at a path that doesn't exist there at all (this file's own
#       whole-repo mirror_source step in CMakeLists.txt deliberately
#       excludes /.git), breaking anything that needs git -- confirmed:
#       MOOSE's own submodule init for libmesh failed exactly this way
#       when its mirrored copy's gitlink resolved to a nonexistent path; or
#   (b) if repointed at the *original* gitdir's absolute path instead,
#       would silently operate on the *original* source tree's working
#       directory rather than the mirror -- that gitdir's own config sets
#       core.worktree to a relative path back to the original checkout,
#       which git honors regardless of where the gitlink pointing at it
#       lives (confirmed directly by inspecting .git/modules/<path>/config
#       for a real Cardinal checkout).
#
# Fix: copy the *real* git directory's contents into <dest>/.git (a real
# directory, not a gitlink) and strip any core.worktree override from its
# config, so the copy is a genuinely self-contained repository whose
# worktree is itself.
#
# Expected -D arguments: IN_SOURCE_DIR, DEST_DIR.
cmake_minimum_required(VERSION 3.21)

if(NOT IN_SOURCE_DIR OR NOT DEST_DIR)
  message(FATAL_ERROR "CardinalMirrorSubmodule.cmake: IN_SOURCE_DIR and DEST_DIR are required")
endif()

file(MAKE_DIRECTORY "${DEST_DIR}")

execute_process(
  COMMAND rsync -rlpgo --update --exclude=.git "${IN_SOURCE_DIR}/" "${DEST_DIR}/"
  RESULT_VARIABLE _rc)
if(_rc)
  message(FATAL_ERROR "Cardinal: failed to mirror '${IN_SOURCE_DIR}' into '${DEST_DIR}'")
endif()

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
# `--file <path>` targets the config file directly rather than going
# through git's normal repo/worktree discovery (`git -C ${DEST_DIR}
# config ...`) -- discovery is exactly what's broken here (core.worktree
# still points at the original checkout at this point), so it would fail
# before ever reaching the --unset. `git config --unset` exits 5 (not 0)
# when the key was never set, which isn't an error here -- e.g.
# IN_SOURCE_DIR wasn't itself a submodule.
execute_process(
  COMMAND git config --file "${DEST_DIR}/.git/config" --unset core.worktree
  ERROR_QUIET)
