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

# Exclude common compiler/linker/libtool byproducts, in addition to .git
# (handled separately below). Unlike mirror_source's own /build and /install
# excludes in CMakeLists.txt, IN_SOURCE_DIR here is a *third-party*
# dependency's own checkout (e.g. contrib/moose) -- if that checkout has
# ever itself been built in place (NO_BUILD dependencies like MOOSE compile
# in-place, with no separate build directory of their own to exclude the
# way Cardinal's own top-level checkout has one), a plain rsync mirrors
# those compiled artifacts right alongside the source. Confirmed live: a
# `contrib/moose` checkout that had separately been natively built (a
# leftover dbg-mode MOOSE build, from unrelated earlier use of that same
# checkout) mirrored its own stale .o/.so/.d files in on top of a fresh
# opt-mode build here, and one of its .d dependency files -- still
# referencing the *original* checkout's absolute paths -- left at least one
# unity translation unit's own object file stale despite the containing
# .so's mtime looking fresh, producing a real, silent undefined-reference
# link failure. These exact patterns (plus `build`/`*Revision.h`, MOOSE's
# own equivalent of Cardinal's own generated CardinalRevision.h -- see the
# "why no --delete" note on mirror_source in CMakeLists.txt) are pulled
# directly from MOOSE's own .gitignore, not guessed: deliberately a
# narrow, high-confidence subset (compiler/linker output only), not the
# broader .gitignore patterns further down that file (*.csv, *.log, *.e,
# ...) --
# mirror_source's own design notes above already found that a broad
# ignore-pattern filter can wrongly exclude a file that's explicitly
# tracked despite matching it (e.g. a fixture tracked despite a blanket
# *.xml ignore); output/data-file patterns are exactly the kind of pattern
# that risk applies to, so they're deliberately left alone here -- only
# patterns no build ever legitimately tracks as source are excluded.
#
# `*.so.[0-9]*`, not `*.so.*`: rsync's exclude patterns match the *whole*
# filename via fnmatch, not just a suffix, so a trailing `*` right after
# ".so" turns this into a substring match rather than an extension match --
# `*.so.*` matched `phy.solidwall_outlet_3eqn.i` (a real, tracked MOOSE test
# input), because it merely *contains* ".so" (from "phy." + "solidwall").
# Confirmed live testing this exact patch against a real dirty checkout
# before relying on it. Requiring a digit immediately after ".so." limits
# the match to an actual versioned-shared-library suffix (.so.0, .so.0.0.0)
# instead of any filename that happens to contain ".so" followed by
# anything.
set(_byproduct_excludes
  --exclude=.libs
  --exclude=*.la
  --exclude=*.lo
  --exclude=*.o
  --exclude=*.a
  --exclude=*.so
  --exclude=*.so.[0-9]*
  --exclude=*.dylib
  --exclude=*.d
  --exclude=build
  --exclude=*Revision.h)

execute_process(
  COMMAND rsync -rlpgo --update --exclude=.git ${_byproduct_excludes} "${IN_SOURCE_DIR}/" "${DEST_DIR}/"
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
#
# Verified (and re-run) rather than blindly trusted: observed once, in a
# real concurrent multi-target build (not reproduced running this same
# script in isolation with the same arguments, so never root-caused), that
# core.worktree survived a re-mirror despite this command apparently
# running -- silently, since the old code swallowed every exit code
# (ERROR_QUIET, no RESULT_VARIABLE at all) including a genuine transient
# failure indistinguishable from "key was never set". Now checks the
# actual result: code 5 is the documented, harmless "wasn't set" case;
# anything else retries once (cheap insurance against exactly the kind of
# one-off failure observed) and, only if that retry *also* doesn't leave
# the config clean, surfaces a warning instead of continuing to fail
# silently.
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
