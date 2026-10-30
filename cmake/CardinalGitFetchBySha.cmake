# NOT CURRENTLY USED -- cardinal_add_submodule_dependency's tier-3 clone path
# (CardinalDependency.cmake) now uses ExternalProject's own built-in
# GIT_REPOSITORY/GIT_TAG support instead, matching scripts/
# get-dependencies.sh's own plain `git submodule update --init`. Kept here
# rather than deleted in case that needs revisiting.
#
# This script fetches one exact commit by SHA (`git fetch <url> <sha>`, the
# same thing `git submodule update --init` does under the hood), which some
# git servers honor even for a commit unreachable from any advertised
# branch/tag -- unlike ExternalProject's own git step, a plain `git clone` +
# `git checkout <sha>`, which only works if that commit is reachable from
# some advertised ref. That only helps while the unreachable commit object
# still physically exists upstream, though: it provides no protection at
# all once that object is garbage-collected, which an orphaned
# (not-on-any-branch) commit is always at some risk of eventually being.
# Pinning to a commit that's reachable from a real branch in the first
# place is the actual fix; see doc/content/cmake_details.md.
#
# Expected -D arguments: GIT_REPOSITORY, GIT_TAG, SOURCE_DIR, INIT_SUBMODULES
# (TRUE/FALSE).
cmake_minimum_required(VERSION 3.21)

if(NOT GIT_REPOSITORY OR NOT GIT_TAG OR NOT SOURCE_DIR)
  message(FATAL_ERROR "CardinalGitFetchBySha.cmake: GIT_REPOSITORY, GIT_TAG, and SOURCE_DIR are required")
endif()

file(REMOVE_RECURSE "${SOURCE_DIR}")
file(MAKE_DIRECTORY "${SOURCE_DIR}")

execute_process(
  COMMAND git init -q .
  WORKING_DIRECTORY "${SOURCE_DIR}"
  RESULT_VARIABLE _rc)
if(_rc)
  message(FATAL_ERROR "Cardinal: failed to 'git init' in '${SOURCE_DIR}'")
endif()

# A real `origin` remote (not a bare-URL fetch) matters beyond just being
# tidy: a submodule's own .gitmodules can point at ITS submodules with a
# relative URL (MOOSE's does, e.g. "../../libMesh/libmesh" for libmesh) --
# git resolves that relative to the parent repo's own origin URL. With no
# origin configured at all, git instead falls back to resolving it against
# the local filesystem path of the current working directory, silently
# producing a bogus local path (observed directly: MOOSE cloned this way,
# then `git submodule update --init libmesh` run from inside it, failed
# with "repository '<up-two-dirs>/libMesh/libmesh' does not exist"). Fetch
# from the named remote, not a bare URL, so relative submodule resolution
# in anything we clone this way works exactly like a normal `git clone`.
execute_process(
  COMMAND git remote add origin "${GIT_REPOSITORY}"
  WORKING_DIRECTORY "${SOURCE_DIR}"
  RESULT_VARIABLE _rc)
if(_rc)
  message(FATAL_ERROR "Cardinal: failed to add remote 'origin' -> '${GIT_REPOSITORY}' in '${SOURCE_DIR}'")
endif()

# --depth 1: we only ever need this exact commit, not its history.
execute_process(
  COMMAND git fetch -q --depth 1 origin "${GIT_TAG}"
  WORKING_DIRECTORY "${SOURCE_DIR}"
  RESULT_VARIABLE _rc)
if(_rc)
  message(FATAL_ERROR "Cardinal: failed to fetch '${GIT_TAG}' from '${GIT_REPOSITORY}'")
endif()

# --detach: matches ExternalProject's own gitclone.cmake, which clones with
# `advice.detachedHead=false` -- we're pinned to a commit, not a branch.
execute_process(
  COMMAND git checkout -q --detach FETCH_HEAD --
  WORKING_DIRECTORY "${SOURCE_DIR}"
  RESULT_VARIABLE _rc)
if(_rc)
  message(FATAL_ERROR "Cardinal: failed to checkout FETCH_HEAD ('${GIT_TAG}') in '${SOURCE_DIR}'")
endif()

if(INIT_SUBMODULES)
  execute_process(
    COMMAND git submodule update --init --recursive
    WORKING_DIRECTORY "${SOURCE_DIR}"
    RESULT_VARIABLE _rc)
  if(_rc)
    message(FATAL_ERROR "Cardinal: failed to update submodules in '${SOURCE_DIR}'")
  endif()
endif()
