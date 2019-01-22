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
OCCA_DIR        ?= $(CONTRIB_DIR)/occa
LIBP_DIR        ?= $(CONTRIB_DIR)/libparanumal
NEK_LIBP_DIR    ?= $(CONTRIB_DIR)/NekGPU/nek-libp
OPENMC_DIR      ?= $(CONTRIB_DIR)/openmc

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

ADDITIONAL_APP_LIBS := -lnek5000 -L$(NEK_CASEDIR) -lopenmc -L$(OPENMC_DIR)/build

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
export OCCA_DIR
export CARDINAL_DIR

LIBELLIPTIC    := $(LIBP_DIR)/solvers/elliptic/libelliptic.a
LIBINS         := $(LIBP_DIR)/solvers/ins/libins.a
LIBPARALMOND   := $(LIBP_DIR)/libs/parAlmond/libparAlmond.a
LIBOGS         := $(LIBP_DIR)/libs/gatherScatter/libogs.a
LIBBLASLAPACK  := $(LIBP_DIR)/3rdParty/BlasLapack/libBlasLapack.a
PARANUMAL_LIBS := $(LIBELLIPTIC) $(LIBINS) $(LIBPARALMOND) $(LIBOGS) $(LIBBLASLAPACK)

occa:
	make -C $(OCCA_DIR) -j

$(PARANUMAL_LIBS): occa
	make -C $(dir $@) -j lib

nek-libp: $(PARANUMAL_LIBS)
	mkdir -p $(NEK_LIBP_DIR)/build
	cd $(NEK_LIBP_DIR)/build && cmake -DBASEDIR=$(CONTRIB_DIR) ..
	make VERBOSE=1 -C $(NEK_LIBP_DIR)/build nek-libp

# Just compiles gslib and libnek5000 (not cvode or hypre)
libnek5000: nek-libp
	echo $(CARDINAL_DIR)
	SOURCE_ROOT="$(NEK_DIR)" \
		CASEDIR="$(NEK_CASEDIR)" \
		CASENAME="$(NEK_CASENAME)" \
	 	FFLAGS="-mcmodel=large -I$(NEK_LIBP_DIR)/inc" \
		CFLAGS="-mcmodel=large" \
		MPI=1 \
		USR_LFLAGS="-mcmodel=large -L$(NEK_LIBP_DIR)/build -Wl,-rpath,$(NEK_LIBP_DIR)/build -lnek-libp" \
		$(CARDINAL_DIR)/config/configure-nek.sh
	cd $(NEK_DIR)/3rd_party/gslib && ./install
	# cd $(NEK_DIR)/3rd_party/cvode && ./install
	# cd $(NEK_DIR)/3rd_party/hypre && ./install
	make -C $(NEK_CASEDIR) lib

libopenmc:
	mkdir -p $(OPENMC_DIR)/build
	cd $(OPENMC_DIR)/build && cmake $(OPENMC_DIR)
	make -C $(OPENMC_DIR)/build libopenmc

.PHONY: occa nek-libp libnek5000 libopenmc
