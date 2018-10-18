###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Optional Environment variables
# MOOSE_DIR        - Root directory of the MOOSE project
#
###############################################################################

CARDINAL_DIR    ?= $(CURDIR)
CONTRIB_DIR     := $(CARDINAL_DIR)/contrib
MOOSE_SUBMODULE ?= $(CONTRIB_DIR)/moose
NEK5K_DIR       ?= $(CONTRIB_DIR)/Nek5000
OCCA_DIR        ?= $(CONTRIB_DIR)/occa
LIBP_DIR        ?= $(CONTRIB_DIR)/libparanumal
NEK_LIBP_DIR    ?= $(CONTRIB_DIR)/NekGPU/nek-libp

################################## MOOSE ####################################

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

################################## MODULES ####################################
# To use certain physics included with MOOSE, set variables below to
# yes as needed.  Or set ALL_MODULES to yes to turn on everything (overrides
# other set variables).

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
###############################################################################

# dep apps
APPLICATION_DIR    := $(CURDIR)
APPLICATION_NAME   := cardinal
BUILD_EXEC         := yes
GEN_REVISION       := no
include            $(FRAMEWORK_DIR)/app.mk

###############################################################################
# Additional special case targets should be added here

export CXX := $(libmesh_CXX)
export CC  := $(libmesh_CC)
export FC  := $(libmesh_FC)

occa: 
	make -C $(OCCA_DIR) -j

libparanumal:
	make -C $(LIBP_DIR)/solvers/elliptic -j
	make -C $(LIBP_DIR)/solvers/ins -j
	make -C $(LIBP_DIR)/solvers/parAlmond -j


.PHONY: occa libparanumal
