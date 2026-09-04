# Forwards to the "cardinal-unit" target in the main Makefile -- the only
# thing this file has ever actually been used for (checking_install.md's own
# instructions are exactly `cd unit; make -j8`); nothing in this repo invokes
# it with any other target name. A generic catch-all forwarder (%: ...) was
# tried instead and rejected: besides needing a FORCE prerequisite to avoid
# silently no-op'ing once unit/cardinal-unit-opt or unit/run_tests already
# exist on disk (a plain %: with no prerequisites is otherwise
# indistinguishable from an ordinary up-to-date file rule), it also matches
# the literal name "Makefile", which -- once given a prerequisite -- trips
# GNU Make's own "remake the makefile" logic and reruns everything twice.
# None of that complexity buys anything real: even "clean" wouldn't forward
# to anything useful, since cardinal-unit's own targets in the main Makefile
# only exist when "cardinal-unit" is itself among the requested goals (see
# its own comments there).
.DEFAULT_GOAL := cardinal-unit

.PHONY: cardinal-unit
cardinal-unit:
	$(MAKE) -C $(CURDIR)/.. $@
