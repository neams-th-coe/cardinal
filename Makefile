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

CARDINAL_DIR    := $(abspath $(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))))
CONTRIB_DIR     := $(CARDINAL_DIR)/contrib
MOOSE_SUBMODULE ?= $(CONTRIB_DIR)/moose
NEK_DIR         ?= $(CONTRIB_DIR)/Nek5000
OPENMC_DIR      ?= $(CONTRIB_DIR)/openmc
GSLIB_DIR       ?= $(CONTRIB_DIR)/gslib

NEK_CASEDIR  ?= $(CARDINAL_DIR)/problems/spherical_heat_conduction
NEK_CASENAME ?= onepebble

# ======================================================================================
# PETSc
# ======================================================================================

# Use compiler info discovered by PETSC
include $(PETSC_DIR)/lib/petsc/conf/petscvariables

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
# MOOSE Apps
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
# ======================================================================================

export CXX := $(libmesh_CXX)
export CC  := $(libmesh_CC)
export FC  := $(libmesh_F90)
export CARDINAL_DIR

APPLICATION_DIR    := $(CARDINAL_DIR)
APPLICATION_NAME   := cardinal-$(NEK_CASENAME)
BUILD_EXEC         := yes
GEN_REVISION       := no

include            $(CARDINAL_DIR)/config/gslib.mk
include            $(CARDINAL_DIR)/config/nek.mk
include            $(CARDINAL_DIR)/config/openmc.mk

# CC_LINKER_SLFLAG is from petscvariables
ADDITIONAL_DEPEND_LIBS := $(NEK_LIB) $(GS_LIB) $(OPENMC_LIB)
ADDITIONAL_LIBS := -L$(NEK_LIBDIR) -L$(GS_LIBDIR) -L$(OPENMC_LIBDIR) -lnek5000_$(NEK_CASENAME) -lgs -lopenmc $(CC_LINKER_SLFLAG)$(NEK_LIBDIR) $(CC_LINKER_SLFLAG)$(GS_LIBDIR) $(CC_LINKER_SLFLAG)$(OPENMC_LIBDIR)
ADDITIONAL_INCLUDES := -I$(CURDIR)/include -I$(OPENMC_DIR)/include
CARDINAL_EXTERNAL_FLAGS := -L$(NEK_LIBDIR) -L$(GS_LIBDIR) -L$(OPENMC_LIBDIR) \
  -lnek5000_$(NEK_CASENAME) -lgs -lopenmc $(CC_LINKER_SLFLAG)$(NEK_LIBDIR) \
  $(CC_LINKER_SLFLAG)$(GS_LIBDIR) $(CC_LINKER_SLFLAG)$(CURDIR)/lib

include            $(FRAMEWORK_DIR)/app.mk

$(app_LIB): EXTERNAL_FLAGS := $(CARDINAL_EXTERNAL_FLAGS)
$(app_test_LIB): EXTERNAL_FLAGS := $(CARDINAL_EXTERNAL_FLAGS)
$(app_EXEC): EXTERNAL_FLAGS := $(CARDINAL_EXTERNAL_FLAGS)
