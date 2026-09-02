# cmake -P script used as a custom ExternalProject_Add DOWNLOAD_COMMAND (see
# the tier-3 clone path in cardinal_add_submodule_dependency,
# CardinalDependency.cmake).
#
# Why this exists rather than just using ExternalProject's own built-in
# GIT_REPOSITORY/GIT_TAG support: that mechanism (gitclone.cmake) does a plain
# `git clone` -- which only fetches objects reachable from an advertised
# branch/tag -- and then `git checkout <sha>`. That fails with "fatal:
# reference is not a tree" whenever the pinned commit isn't at the tip of any
# branch upstream, which happens for real: Cardinal's own contrib/moab pin
# (bcf15c8ed0...) is exactly such a commit on the fathomteam/moab Bitbucket
# repo, confirmed by a plain `git clone` of the full repo (all branches) not
# containing the object at all.
#
# `git submodule update --init` succeeds on that same commit because git
# instead does a direct `git fetch <url> <sha>`, which most servers honor
# even for objects unreachable from any ref (verified here for Bitbucket and
# GitHub) as long as the object exists at all. This script does the same:
# fetch the exact commit by SHA, then check it out -- no reliance on it being
# reachable from a branch/tag.
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
