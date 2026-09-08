# Forwards to the "cardinal-unit" target in the main Makefile -- the only
# thing this file has ever actually been used for (checking_install.md's own
# instructions are exactly `cd unit; make -j8`); nothing in this repo invokes
# it with any other target name. Deliberately generic (unaware of how
# cardinal-unit actually gets built -- see the main Makefile's own comments
# on that) so the same file works unchanged under any build system that
# defines an equivalent "cardinal-unit" target (e.g. the CMake superbuild).
#
# A generic catch-all forwarder (%: ...) was tried instead of a named
# "cardinal-unit" target specifically and rejected: besides needing a FORCE
# prerequisite to avoid silently no-op'ing once unit/cardinal-unit-opt or
# unit/run_tests already exist on disk (a plain %: with no prerequisites is
# otherwise indistinguishable from an ordinary up-to-date file rule), it also
# matches the literal name "Makefile", which -- once given a prerequisite --
# trips GNU Make's own "remake the makefile" logic and reruns everything
# twice. None of that complexity buys anything real: even "clean" wouldn't
# forward to anything useful, since the main Makefile only knows how to
# clean whichever single app it's currently building.
.DEFAULT_GOAL := cardinal-unit

.PHONY: cardinal-unit
cardinal-unit:
	$(MAKE) -C $(CURDIR)/.. $@
