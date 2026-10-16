# cmake -P script used as mirror_source's actual build-time command (see
# CMakeLists.txt) -- rather than a literal rsync command baked directly
# into the custom target's argument string at *configure* time, so that
# the exclude list below is computed fresh on every single invocation.
#
# That matters for one specific reason: part of what this script excludes
# (see "Other CMake build directories" below) is *other CMake build
# directories sitting next to this one* -- a set that can change for as
# long as this build directory exists. A `build5` created after this
# build directory (say, `build4`) was configured still needs to be
# excluded the next time build4's mirror_source target runs, and an
# exclude list fixed into the custom command's string at generate time
# could never see it -- it would copy build5 straight into build4 on the
# next invocation. Recomputing the whole list as part of the command
# itself, instead of baking it into the command's own argument string,
# means every run reflects the real, current state of the source tree.
#
# Expected -D arguments: SOURCE_DIR, DEST_DIR, ENABLE_NEK (ON/OFF).
cmake_minimum_required(VERSION 3.21)

if(NOT SOURCE_DIR OR NOT DEST_DIR)
  message(FATAL_ERROR "CardinalMirrorSource.cmake: SOURCE_DIR and DEST_DIR are required")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/CardinalByproductExcludes.cmake)

# -----------------------------------------------------------------------
# Fixed excludes
# -----------------------------------------------------------------------
#
# --exclude=/contrib covers all 14 submodules under it (the 4 this
# superbuild manages -- moose/nekRS/openmc/nuclear_data, each resolved
# independently by its own cardinal_add_submodule_dependency call -- plus
# 10 more for not-yet-implemented Phase 2 features like DAGMC/SAM/bison/
# embree/... that we don't want mirrored at all even if a user happens to
# have them checked out locally). Confirmed nothing else lives directly
# under contrib/ -- every path there is a submodule mount point, nothing
# Cardinal-specific.
#
# --exclude=/Makefile: DEST_DIR is the CMake build directory itself, so
# rsync-ing Makefile normally would collide with CMake's own generated
# one of the same name. CMakeLists.txt separately copy_if_different's it
# to Makefile.cardinal instead.
#
# --exclude=/unit/Makefile: same collision-avoidance idea, but for a
# different reason -- unit/Makefile doesn't collide with anything CMake
# generates, but it has its own independent set of ENABLE_NEK/
# ENABLE_OPENMC/ENABLE_DAGMC/ENABLE_DOUBLE_DOWN/PETSC_DIR/LIBMESH_DIR/
# WASP_DIR/HDF5_ROOT `?=` defaults that must match how *this* build was
# actually configured, or its own build_openmc/etc steps silently rebuild
# those dependencies with different settings (confirmed directly:
# invoking it with the wrong ENABLE_DAGMC rebuilt libopenmc.so without
# DAGMC support, breaking the link of the already-built, DAGMC-enabled
# libcardinal-opt.so with "undefined reference to
# openmc::DAGSurface/DAGUniverse"). CMakeLists.txt separately
# copy_if_different's it to unit/Makefile.unit, and replaces unit/Makefile
# itself with a tiny static forwarder (cmake/UnitMakefile) that forces
# those settings from this build's own cached configuration first.
#
# --exclude=/build --exclude=/install: Cardinal's own native Makefile
# defaults both to exactly these names directly under the source checkout
# (CARDINAL_DIR/build, CARDINAL_DIR/install -- the same *_BUILDDIR/
# *_INSTALL_DIR paths this superbuild's own dependency blocks are careful
# to match). Since DEST_DIR *is* the CMake build directory, a bare rsync
# of the whole source tree into it would, for anyone who has ever run (or
# runs concurrently) a native build in that same checkout, copy that
# leftover/live build/install tree wholesale on top of our own
# ExternalProject build trees living at those same relative paths --
# silently corrupting them (observed directly: a stale
# build/openmc/CMakeCache.txt from an earlier native build clobbered a
# freshly-configured ExternalProject build/openmc here). Excluded
# unconditionally, not just when such a directory happens to exist.
#
# These two can't be derived the same way as "Other CMake build
# directories" below: `build` is a plain container directory with no
# CMakeCache.txt of its own (each dependency's own CMake sub-build --
# build/openmc, build/nekrs, ... -- has one a level deeper, which a check
# of build/ itself would never see), and `install` is never a CMake
# directory at all. Both have to stay as fixed, named excludes.
#
# Byproduct excludes (CARDINAL_BYPRODUCT_EXCLUDES, included above): a
# native Makefile build doesn't confine itself to /build and /install at
# all -- Cardinal builds in-place, with object files, libraries,
# dependency files, and *Revision.h landing inline next to their sources
# (e.g. src/base/CardinalApp.o), exactly the same in-place-compilation
# shape CardinalMirrorSubmodule.cmake already has to guard against for
# contrib/moose. Without this, a native build run directly in this
# checkout would mirror its own stale .o/.d/*Revision.h files into this
# build's stage tree the same way already confirmed to happen there.
#
# --exclude=/CMakeLists.txt --exclude=/cmake: this superbuild's own files
# (this directory's parent CMakeLists.txt, and the helper modules under
# cmake/, including this script). Cardinal's native Makefile has no use
# for either -- they exist purely to drive this CMake configure -- so
# mirroring them into DEST_DIR (which, again, is the CMake build
# directory itself) would just leave a redundant, easily stale copy
# sitting next to CMake's own CMakeCache.txt/CMakeFiles for no purpose.
set(_excludes
  --exclude=/contrib
  --exclude=/.git
  --exclude=/Makefile
  --exclude=/unit/Makefile
  --exclude=/build
  --exclude=/install
  --exclude=/CMakeLists.txt
  --exclude=/cmake
  ${CARDINAL_BYPRODUCT_EXCLUDES})

if(ENABLE_NEK)
  # test/tests/nek_ci is its own submodule outside contrib/, resolved
  # independently (like contrib's own submodules) only when ENABLE_NEK is
  # on. Exclude it here too in that case so the two don't fight over the
  # same destination; when ENABLE_NEK is off there's no dedicated
  # resolver for it, so let it just get copied normally along with
  # everything else (a no-op if it isn't checked out).
  list(APPEND _excludes --exclude=/test/tests/nek_ci)
endif()

# -----------------------------------------------------------------------
# Other CMake build directories
# -----------------------------------------------------------------------
#
# Catches two cases with one rule, computed fresh on every invocation
# (see the file header):
#
#  1. This build directory mirroring itself into itself
#     (build1/build1/build1/... forever) whenever it isn't literally
#     named "build" -- the fixed exclude above only ever protected that
#     one conventional name. CMake always writes CMakeCache.txt into the
#     binary directory during *configure*, strictly before this script
#     ever runs (configure always completes, including writing
#     CMakeCache.txt, before any build-time custom command executes) --
#     so by the time this script runs, DEST_DIR already has its own
#     CMakeCache.txt and is correctly caught by this same scan, with no
#     special-casing of "the current build directory" needed at all.
#
#  2. Any other, differently-named CMake build directory left sitting
#     next to this one -- e.g. build1 configured, then build2, then
#     build3: each would otherwise get mirrored wholesale into whichever
#     one builds next.
#
# Only scans immediate children of SOURCE_DIR, not the whole tree
# recursively: every build directory in this workflow is created
# directly under the repo root (`cmake -S . -B <name>`), and a deep
# recursive scan would be slower and would need its own logic to avoid
# descending into the fixed excludes above anyway.
file(GLOB _top_level_entries LIST_DIRECTORIES true ${SOURCE_DIR}/*)
foreach(_entry ${_top_level_entries})
  if(IS_DIRECTORY ${_entry} AND EXISTS ${_entry}/CMakeCache.txt)
    get_filename_component(_name ${_entry} NAME)
    list(APPEND _excludes --exclude=/${_name})
  endif()
endforeach()

# -----------------------------------------------------------------------
# The mirror itself
# -----------------------------------------------------------------------
#
# -rlpgo, not -a: -a is -rlptgoD, and the -t (preserve source mtime) is
# exactly wrong here. The source tree can keep being edited while a long
# build runs, and Cardinal's Makefile relies on ordinary mtime
# comparisons to decide what needs recompiling -- copying a source mtime
# through verbatim (rather than stamping the destination with the actual
# copy time) is what copy_directory_if_newer avoids too. -D
# (devices/specials) is dropped as well since it's meaningless for a
# source tree.
#
# --update is required alongside dropping -t, not optional: without -t,
# the destination's mtime is just "whenever it was last copied", which
# will essentially never equal the source's current mtime -- so rsync's
# default quick check (size+mtime) sees every file as changed and
# re-copies (and re-stamps) the entire tree on every single build,
# forcing a full recompile+relink every time regardless of whether
# anything actually changed. --update skips a file whenever the
# destination is already newer than the source (only comparing
# timestamps, not hashing every file's content) -- a genuinely-unchanged
# file keeps its stable destination mtime, and a real source edit (whose
# mtime is necessarily newer than the last mirror copy) gets re-copied
# with a fresh one.
#
# No --delete: Cardinal builds in-place (see the byproduct-excludes note
# above), so a bare --delete would remove every one of those generated
# files from the mirror as soon as they're produced -- forcing a full
# rebuild+relink every time regardless of the mtime/--update fix above
# (confirmed live: CardinalRevision.h was getting deleted and
# regenerated on every build; a missing prerequisite is obviously
# "changed", no mtime trickery needed for that). The cost of skipping
# --delete is a stale copy lingering if a source file is later
# removed/renamed upstream, far cheaper than silently rebuilding the
# world every time.
#
# Just copy everything else (only excluding what's handled above),
# rather than trying to filter by .gitignore patterns: that was tried and
# got it wrong in a way that broke tests -- .gitignore only hides
# *untracked* files from git, so a file that's explicitly tracked despite
# matching a broad ignore pattern (e.g. test/tests/.../model.xml, tracked
# despite the repo's blanket "*.xml" ignore meant to keep *generated* XML
# out, not this fixture) stays tracked and visible to git regardless, but
# rsync's --filter has no such "tracked overrides ignore" concept and
# excluded it unconditionally.
file(MAKE_DIRECTORY ${DEST_DIR})

execute_process(
  COMMAND rsync -rlpgo --update ${_excludes} ${SOURCE_DIR}/ ${DEST_DIR}/
  RESULT_VARIABLE _rc)
if(_rc)
  message(FATAL_ERROR "Cardinal: failed to mirror '${SOURCE_DIR}' into '${DEST_DIR}'")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/CardinalDetectMirrorDivergence.cmake)
cardinal_detect_mirror_divergence(${SOURCE_DIR} ${DEST_DIR} ${_excludes})
