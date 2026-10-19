# Shared rsync --exclude list for compiler/linker byproducts that a
# MOOSE-style Makefile build -- Cardinal's own top-level checkout, or an
# in-source third-party submodule like contrib/moose -- scatters directly
# next to its sources rather than confining to a separate build
# directory. Included by both CardinalMirrorSource.cmake and
# CardinalMirrorSubmodule.cmake so the two can't drift apart. See
# doc/content/cmake_details.md for the full rationale, including why a
# directory-name pattern (e.g. a bare "build") doesn't belong here even
# though it would be valid for some individual submodules.
#
# `*.so.[0-9]*`, not `*.so.*`: rsync's exclude patterns match the *whole*
# filename via fnmatch, not just a suffix, so a trailing `*` right after
# ".so" turns this into a substring match rather than an extension match
# -- it would also match any filename that merely contains ".so".
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
