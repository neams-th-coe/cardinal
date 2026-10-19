# Shared rsync --exclude list for compiler/linker byproducts that a
# MOOSE-style Makefile build -- Cardinal's own top-level checkout, or an
# in-source third-party submodule like contrib/moose -- scatters directly
# next to its sources rather than confining to a separate build
# directory: object files, archives, generated *Revision.h headers, etc.
# Anything that rsync-mirrors such a checkout (CardinalMirrorSource.cmake
# for Cardinal's own tree, CardinalMirrorSubmodule.cmake for in-source
# submodules) must exclude these explicitly, or a prior native build run
# in that same checkout gets mirrored right on top of a fresh build.
#
# Confirmed live (via CardinalMirrorSubmodule.cmake, mirroring a
# natively-built contrib/moose checkout): without these excludes, stale
# .o/.so/.d files got copied in on top of a fresh opt-mode build, and one
# stale .d dependency file -- still referencing the *original* checkout's
# absolute paths -- left a unity translation unit's own object file
# silently stale despite its containing .so's mtime looking fresh,
# producing a real, silent undefined-reference link failure.
#
# Kept in one shared file, included by both mirror scripts, rather than
# duplicated in each, so the two copies can't drift apart.
#
# `*.so.[0-9]*`, not `*.so.*`: rsync's exclude patterns match the *whole*
# filename via fnmatch, not just a suffix, so a trailing `*` right after
# ".so" turns this into a substring match rather than an extension match --
# `*.so.*` matched `phy.solidwall_outlet_3eqn.i` (a real, tracked MOOSE test
# input), because it merely *contains* ".so" (from "phy." + "solidwall").
# Confirmed live against a real dirty checkout before relying on it.
#
# No bare `--exclude=build` here, deliberately: an earlier version of this
# list had one (borrowed verbatim from MOOSE's own .gitignore, which really
# does ignore any directory named "build" anywhere in its tree -- e.g. its
# own conda/build). That's fine for MOOSE specifically, but this list is
# shared across *every* in-source submodule mirrored via
# CardinalMirrorSubmodule.cmake, and rsync's unanchored "build" pattern
# matches a directory of that name at *any* depth, in *any* submodule --
# not just MOOSE's own disposable ones. Confirmed live: it silently
# excluded contrib/nekRS's vendored OCCA's own tracked
# `3rd_party/occa/scripts/build/` directory, breaking NekRS's CMake
# configure outright ("File .../compiledDefinesTemplate.hpp.in does not
# exist"). Only the universal, extension-based compiler/linker-output
# patterns below are safe to share across arbitrary submodules; a
# directory-name heuristic borrowed from one specific submodule's own
# .gitignore is not.
set(CARDINAL_BYPRODUCT_EXCLUDES
  --exclude=.libs
  --exclude=*.la
  --exclude=*.lo
  --exclude=*.o
  --exclude=*.a
  --exclude=*.so
  --exclude=*.so.[0-9]*
  --exclude=*.dylib
  --exclude=*.d
  --exclude=*Revision.h)
