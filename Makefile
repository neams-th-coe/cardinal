# ======================================================================================
# Cardinal Makefile
# ======================================================================================
#
# Required environment Variables:
# * NEK_CASEDIR : Dir with Nek5000 input deck (.usr, SIZE, and .rea files)
# * NEK_CASENAME : Name of the Nek5000 .usr and .rea files
# 
# Optional environment variables:
# * CARDINAL_DIR : Top-level Cardinal src dir (default: this Makefile's dir)
# * CONTRIB_DIR : Dir with third-party src (default: $(CARDINAL_DIR)/contrib)
# * MOOSE_SUBMODULE : Top-level MOOSE src dir (default: $(CONTRIB_DIR)/moose)
# * NEK5_DIR : Top-level Nek5000 src dir (default: $(CONTRIB_DIR)/Nek5000)
# * OCCA_DIR : Top-level OCCA src dir (default: $(CONTRIB_DIR)/occa)
# * LIBP_DIR : Top-level libparanumal src dir (default: $(CONTRIB_DIR)/libparanumal 
# * NEK_LIBP_DIR : Top-level nek-libp src dir (default: $(CONTRIB_DIR)/NekGPU/nek-libp
#
# ======================================================================================

CARDINAL_DIR := $(abspath $(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))))
CONTRIB_DIR     := $(CARDINAL_DIR)/contrib
MOOSE_SUBMODULE ?= $(CONTRIB_DIR)/moose
NEK_DIR         ?= $(CONTRIB_DIR)/Nek5000
OPENMC_DIR      ?= $(CONTRIB_DIR)/openmc

# DEBUG
NEK_CASEDIR  ?= $(NEK_DIR)/short_tests/ethier/
NEK_CASENAME ?= ethier

# ======================================================================================
# MOOSE
# ======================================================================================

# Use the MOOSE submodule if it exists and MOOSE_DIR is not set
ifneq ($(wildcard $(MOOSE_SUBMODULE)/framework/Makefile),)
  MOOSE_DIR        ?= $(MOOSE_SUBMODULE)
else
  MOOSE_DIR        ?= $(shell dirname `pwd`)/moose
endif

# framework
FRAMEWORK_DIR      := $(MOOSE_DIR)/framework
include $(FRAMEWORK_DIR)/build.mk
include $(FRAMEWORK_DIR)/moose.mk

# ======================================================================================
# MOOSE
#
# To use certain physics included with MOOSE, set variables below to
# yes as needed.  Or set ALL_MODULES to yes to turn on everything (overrides
# other set variables).
# ======================================================================================

ALL_MODULES         := no

CHEMICAL_REACTIONS  := no
CONTACT             := no
FLUID_PROPERTIES    := no
HEAT_CONDUCTION     := yes
MISC                := no
NAVIER_STOKES       := no
PHASE_FIELD         := no
RDG                 := no
RICHARDS            := no
SOLID_MECHANICS     := no
STOCHASTIC_TOOLS    := no
TENSOR_MECHANICS    := no
XFEM                := no
POROUS_FLOW         := no

include $(MOOSE_DIR)/modules/modules.mk

# ======================================================================================
# Dependent Apps
#
# To use certain physics included with MOOSE, set variables below to
# yes as needed.  Or set ALL_MODULES to yes to turn on everything (overrides
# other set variables).
# ======================================================================================

APPLICATION_DIR    := $(CARDINAL_DIR)
APPLICATION_NAME   := cardinal
BUILD_EXEC         := yes
GEN_REVISION       := no

$(NEK_CASEDIR)/$(NEK_CASENAME).o: libnek5000

ADDITIONAL_APP_DEPS := libnek5000 libopenmc
ADDITIONAL_LIBS :=  $(NEK_CASEDIR)/obj/$(NEK_CASENAME).o -lnek5000 -L$(NEK_CASEDIR) -lgs -L$(NEK_DIR)/3rd_party/gslib/lib -lopenmc -L$(OPENMC_DIR)/build/lib

include            $(FRAMEWORK_DIR)/app.mk

# ======================================================================================
# Third-party Apps
#
# To use certain physics included with MOOSE, set variables below to
# yes as needed.  Or set ALL_MODULES to yes to turn on everything (overrides
# other set variables).
# ======================================================================================

export CXX := $(libmesh_CXX)
export CC  := $(libmesh_CC)
export FC  := $(libmesh_F90)
export CARDINAL_DIR

libnek5000:
	cd $(NEK_CASEDIR) && SOURCE_ROOT=$(NEK_DIR) $(NEK_DIR)/bin/makenek $(NEK_CASENAME)
	cd $(NEK_CASEDIR) && make lib

libopenmc:
	mkdir -p $(OPENMC_DIR)/build
	cd $(OPENMC_DIR)/build && cmake $(OPENMC_DIR)
	make -C $(OPENMC_DIR)/build libopenmc

.PHONY: occa nek-libp libnek5000 libopenmc
