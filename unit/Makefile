# Builds cardinal-unit: cardinal's own gtest-based unit tests. Two
# app.mk inclusions in this one process -- first for "cardinal" itself
# (its own library, so cardinal-unit can link against it), then for
# "cardinal-unit" -- the same pattern MOOSE's own app-scaffolding
# template (stork/unit/Makefile.app) and MOOSE's own framework unit
# tests (../unit/Makefile) use. app.mk's own bookkeeping variables
# (app_INCLUDES, depend_libs, app_LIBS, ...) accumulate (+=) across
# inclusions in the same process rather than resetting, so cardinal's
# own headers/library are automatically visible to the second
# (cardinal-unit) inclusion with no extra bridging needed.
#
# All dependency/build configuration (NekRS/OpenMC/DAGMC include paths
# and libs, MOOSE module selection, ...) comes from ../Makefile itself,
# including its own bare "cardinal" app.mk inclusion (library only --
# CARDINAL_UNIT_INCLUDE tells it to skip setting BUILD_EXEC/GEN_REVISION/
# DEP_APPS/INSTALLABLE_DIRS, so that inclusion never tries to build
# cardinal's own executable) and its own app_objects/EXTERNAL_FLAGS
# wiring for that bare inclusion's own targets (harmless and correct to
# leave unguarded there -- redoing the same lines below, after our own
# later inclusion, targets cardinal-unit's own, different, reassigned
# app_LIB/app_test_LIB/app_EXEC, not the same ones again). This is a
# same-process include, not a recursive $(MAKE) -- avoids hand-copying
# (and drifting) ../Makefile's dependency configuration a second time,
# without any of the header-visibility/NEKRS_HOME/recursive-process
# concerns a separate `$(MAKE) -C ..` invocation would raise.
#
# ../Makefile by default -- overridable (?=) since that name isn't always
# Cardinal's real top-level Makefile: the CMake superbuild mirrors this
# file in alongside a CMake-generated build-system Makefile of its own,
# which needs the bare name "Makefile" for itself, so Cardinal's real one
# lands as Makefile.cardinal there instead, and that superbuild's own
# CMakeLists.txt passes CARDINAL_MAKEFILE=../Makefile.cardinal explicitly
# when it invokes this file rather than have us guess from what happens
# to exist on disk.
CARDINAL_MAKEFILE ?= ../Makefile
CARDINAL_UNIT_INCLUDE := yes
include $(CARDINAL_MAKEFILE)

APPLICATION_DIR    := $(CURDIR)
APPLICATION_NAME   := cardinal-unit
BUILD_EXEC         := yes

# Extra stuff for GTEST
ADDITIONAL_INCLUDES += -I$(FRAMEWORK_DIR)/contrib/gtest
ADDITIONAL_LIBS     += $(FRAMEWORK_DIR)/contrib/gtest/libgtest.la

include            $(FRAMEWORK_DIR)/app.mk

# app_objects are defined in moose.mk and built according to the rules in build.mk
# We need to build these first so we get include dirs
$(app_objects): build_nekrs build_moab build_embree build_doubledown build_dagmc build_openmc
$(test_objects): build_nekrs build_moab build_embree build_doubledown build_dagmc build_openmc

# EXTERNAL_FLAGS are for rules in app.mk
$(app_LIB): EXTERNAL_FLAGS := $(CARDINAL_EXTERNAL_FLAGS)
$(app_test_LIB): EXTERNAL_FLAGS := $(CARDINAL_EXTERNAL_FLAGS)
$(app_EXEC): EXTERNAL_FLAGS := $(CARDINAL_EXTERNAL_FLAGS)

# Find all the cardinal unit test source files and include their dependencies.
cardinal_unit_srcfiles := $(shell find $(CURRENT_DIR)/src -name "*.C")
cardinal_unit_deps := $(patsubst %.C, %.$(obj-suffix).d, $(cardinal_unit_srcfiles))
-include $(cardinal_unit_deps)
