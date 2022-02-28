# ======================================================================================
# Cardinal Makefile
# ======================================================================================
#
# Optional environment variables:
#
# * CARDINAL_DIR : Top-level Cardinal src dir (default: this Makefile's dir)
# * CONTRIB_DIR : Dir with third-party src (default: $(CARDINAL_DIR)/contrib)
# * HDF5_INCLUDE_DIR: Top-level HDF5 header dir (default: $(HDF5_ROOT)/include)
# * HDF5_LIBDIR: Top-level HDF5 lib dir (default: $(HDF5_ROOT)/lib)
# * HYPRE_DIR: Top-level HYPRE dir (default: $(PETSC_DIR)/$(PETSC_ARCH))
# * MOOSE_SUBMODULE : Top-level MOOSE src dir (default: $(CONTRIB_DIR)/moose)
# * NEKRS_DIR: Top-level NekRS src dir (default: $(CONTRIB_DIR)/nekRS)
# * OPENMC_DIR: Top-level OpenMC src dir (default: $(CONTRIB_DIR)/openmc)
# * PETSC_DIR: Top-levle PETSc src dir (default: $(MOOSE_SUBMODULE)/petsc)
# * PETSC_ARCH: PETSc architecture (default: arch-moose)
# * SAM_DIR: Top-level SAM src dir (default: $(CONTRIB_DIR)/SAM)
# * SOCKEYE_DIR: Top-level Sockeye src dir (default: $(CONTRIB_DIR)/sockeye)
# * SODIUM_DIR: Top-level sodium src dir (default: $(CONTRIB_DIR)/sodium)
# * POTASSIUM_DIR: Top-level potassium src dir (default: $(CONTRIB_DIR)/potassium)
# * IAPWS95_DIR: Top-level iapws95 src dir (default: $(CONTRIB_DIR)/iapws95)
#
# ======================================================================================

# Whether you want to build with NekRS; if not set to 'yes', then you will skip
# the Nek part of Cardinal
ENABLE_NEK        ?= yes

CARDINAL_DIR        := $(abspath $(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))))
CONTRIB_DIR         := $(CARDINAL_DIR)/contrib
HDF5_INCLUDE_DIR    ?= $(HDF5_ROOT)/include
HDF5_LIBDIR         ?= $(HDF5_ROOT)/lib
MOOSE_SUBMODULE     ?= $(CONTRIB_DIR)/moose
NEKRS_DIR           ?= $(CONTRIB_DIR)/nekRS
OPENMC_DIR          ?= $(CONTRIB_DIR)/openmc
PETSC_DIR           ?= $(MOOSE_SUBMODULE)/petsc
PETSC_ARCH          ?= arch-moose
LIBMESH_DIR         ?= $(MOOSE_SUBMODULE)/libmesh/installed/
HYPRE_DIR           ?= $(PETSC_DIR)/$(PETSC_ARCH)
CONTRIB_INSTALL_DIR ?= $(CARDINAL_DIR)/install
SAM_DIR             ?= $(CONTRIB_DIR)/SAM
SOCKEYE_DIR         ?= $(CONTRIB_DIR)/sockeye
SODIUM_DIR          ?= $(CONTRIB_DIR)/sodium
POTASSIUM_DIR       ?= $(CONTRIB_DIR)/potassium
IAPWS95_DIR         ?= $(CONTRIB_DIR)/iapws95

ALL_MODULES         := no

FLUID_PROPERTIES    := yes
HEAT_CONDUCTION     := yes
NAVIER_STOKES       := yes
REACTOR             := yes
STOCHASTIC_TOOLS    := yes
TENSOR_MECHANICS    := yes
THERMAL_HYDRAULICS  := yes

# First, we can find which submodules have been pulled in
MOOSE_CONTENT     := $(shell ls $(MOOSE_DIR) 2> /dev/null)
NEKRS_CONTENT     := $(shell ls $(NEKRS_DIR) 2> /dev/null)
OPENMC_CONTENT    := $(shell ls $(OPENMC_DIR) 2> /dev/null)
SAM_CONTENT       := $(shell ls $(SAM_DIR) 2> /dev/null)
SOCKEYE_CONTENT   := $(shell ls $(SOCKEYE_DIR) 2> /dev/null)

ifeq ($(THERMAL_HYDRAULICS), yes)
  THM_CONTENT     := true
endif

SODIUM_CONTENT    := $(shell ls $(SODIUM_DIR) 2> /dev/null)
POTASSIUM_CONTENT := $(shell ls $(POTASSIUM_DIR) 2> /dev/null)
IAPWS95_CONTENT   := $(shell ls $(IAPWS95_DIR) 2> /dev/null)

# Print errors if some submodules are missing or various pre-reqs for Sockeye,
# SAM, and THM optional submodules are missing or conflict with one another
include config/check_deps.mk

# BUILD_TYPE will be passed to CMake via CMAKE_BUILD_TYPE
ifeq ($(METHOD),dbg)
	BUILD_TYPE := Debug
else
	BUILD_TYPE := Release
endif

OCCA_CUDA_ENABLED=0
OCCA_HIP_ENABLED=0
OCCA_OPENCL_ENABLED=0

NEKRS_BUILDDIR := $(CARDINAL_DIR)/build/nekrs
NEKRS_INSTALL_DIR := $(CONTRIB_INSTALL_DIR)
NEKRS_INCLUDES := \
	-I$(NEKRS_DIR)/src \
	-I$(NEKRS_DIR)/src/cds \
	-I$(NEKRS_DIR)/src/core \
	-I$(NEKRS_DIR)/src/core/utils \
	-I$(NEKRS_DIR)/src/elliptic \
	-I$(NEKRS_DIR)/src/elliptic/linearSolver \
	-I$(NEKRS_DIR)/src/elliptic/amgSolver \
	-I$(NEKRS_DIR)/src/elliptic/amgSolver/amgx \
	-I$(NEKRS_DIR)/src/elliptic/amgSolver/hypre \
	-I$(NEKRS_DIR)/src/elliptic/amgSolver/parAlmond \
	-I$(NEKRS_DIR)/src/elliptic/amgSolver/parAlmond/agmgSetup \
	-I$(NEKRS_DIR)/src/io \
	-I$(NEKRS_DIR)/src/lib \
	-I$(NEKRS_DIR)/src/linAlg \
	-I$(NEKRS_DIR)/src/lns \
	-I$(NEKRS_DIR)/src/mesh \
	-I$(NEKRS_DIR)/src/nekInterface \
	-I$(NEKRS_DIR)/src/plugins \
	-I$(NEKRS_DIR)/src/regularization \
	-I$(NEKRS_DIR)/src/timeStepper \
	-I$(NEKRS_DIR)/src/udf \
	-I$(NEKRS_INSTALL_DIR)/gatherScatter \
	-I$(NEKRS_INSTALL_DIR)/include \
	-I$(NEKRS_INSTALL_DIR)/libparanumal/include \
	-I$(NEKRS_INSTALL_DIR)/include/libP/parAlmond \
	-I$(NEKRS_INSTALL_DIR)/include/linAlg
NEKRS_LIBDIR := $(NEKRS_INSTALL_DIR)/lib
NEKRS_LIB := $(NEKRS_LIBDIR)/libnekrs.so
# This needs to be exported
export NEKRS_HOME=$(CARDINAL_DIR)

OPENMC_BUILDDIR := $(CARDINAL_DIR)/build/openmc
OPENMC_INSTALL_DIR := $(CONTRIB_INSTALL_DIR)
OPENMC_INCLUDES := -I$(OPENMC_INSTALL_DIR)/include
OPENMC_LIBDIR := $(OPENMC_INSTALL_DIR)/lib
OPENMC_LIB := $(OPENMC_LIBDIR)/libopenmc.so

# This is used in $(FRAMEWORK_DIR)/build.mk
HDF5_INCLUDES       := -I$(HDF5_INCLUDE_DIR) -I$(HDF5_ROOT)/include
ADDITIONAL_CPPFLAGS := $(HDF5_INCLUDES) $(OPENMC_INCLUDES)

ifeq ($(ENABLE_NEK), yes)
  ADDITIONAL_CPPFLAGS += $(NEKRS_INCLUDES)
  libmesh_CXXFLAGS    += -DENABLE_NEK_COUPLING
endif

# ======================================================================================
# PETSc
# ======================================================================================

# Use compiler info discovered by PETSC
ifeq ($(PETSC_ARCH),)
	include $(PETSC_DIR)/$(PETSC_ARCH)/lib/petsc/conf/petscvariables
else
	include $(PETSC_DIR)/lib/petsc/conf/petscvariables
endif

# ======================================================================================
# MOOSE core objects
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
# MOOSE modules
# ======================================================================================

include $(MOOSE_DIR)/modules/modules.mk

# SAM submodule
ifneq ($(SAM_CONTENT),)
  libmesh_CXXFLAGS    += -DENABLE_SAM_COUPLING
  APPLICATION_DIR     := $(SAM_DIR)
  APPLICATION_NAME    := sam
  TENSOR_MECHANICS    := yes
  include             $(FRAMEWORK_DIR)/app.mk
endif

# Sockeye submodule
ifneq ($(SOCKEYE_CONTENT),)
  libmesh_CXXFLAGS    += -DENABLE_SOCKEYE_COUPLING
  APPLICATION_DIR     := $(SOCKEYE_DIR)
  APPLICATION_NAME    := sockeye
  include             $(FRAMEWORK_DIR)/app.mk
endif

# THM submodule
ifneq ($(THM_CONTENT),)
  libmesh_CXXFLAGS    += -DENABLE_THM_COUPLING
endif

# sodium submodule
ifneq ($(SODIUM_CONTENT),)
  libmesh_CXXFLAGS    += -DENABLE_SODIUM
  APPLICATION_DIR     := $(SODIUM_DIR)
  APPLICATION_NAME    := sodium
  include             $(FRAMEWORK_DIR)/app.mk
  include             $(SODIUM_DIR)/libSodium.mk
endif

# potassium submodule
ifneq ($(POTASSIUM_CONTENT),)
  libmesh_CXXFLAGS    += -DENABLE_POTASSIUM
  APPLICATION_DIR     := $(POTASSIUM_DIR)
  APPLICATION_NAME    := potassium
  include             $(FRAMEWORK_DIR)/app.mk
  include             $(POTASSIUM_DIR)/libPotassium.mk
endif

# iapws95 submodule
ifneq ($(IAPWS95_CONTENT),)
  libmesh_CXXFLAGS    += -DENABLE_IAPWS95
  APPLICATION_DIR     := $(IAPWS95_DIR)
  APPLICATION_NAME    := iapws95
  include             $(FRAMEWORK_DIR)/app.mk
  include             $(IAPWS95_DIR)/libSBTL.mk
endif

# ======================================================================================
# External apps
# ======================================================================================

# libmesh_CXX, etc, were defined in build.mk
export CXX := $(libmesh_CXX)
export CC  := $(libmesh_CC)
export FC  := $(libmesh_F90)
export FFLAGS := $(libmesh_FFLAGS)
export CFLAGS := $(libmesh_CFLAGS)
export CXXFLAGS := $(libmesh_CXXFLAGS)
export CPPFLAGS := $(libmesh_CPPFLAGS)
export LDFLAGS := $(libmesh_LDFLAGS)
export LIBS := $(libmesh_LIBS)

CXXFLAGS += -DNEKRS_VERSION=21
CXXFLAGS += -DNEKRS_SUBVERSION=1
CXXFLAGS += -DGITCOMMITHASH=\"51d5bf5f2042e231d1770400c160d5623b19b4c8\"

export CARDINAL_DIR

APPLICATION_DIR    := $(CARDINAL_DIR)
APPLICATION_NAME   := cardinal
BUILD_EXEC         := yes
GEN_REVISION       := no
DEP_APPS           := $(shell $(FRAMEWORK_DIR)/scripts/find_dep_apps.py $(APPLICATION_NAME))

ifeq ($(ENABLE_NEK), yes)
  include            $(CARDINAL_DIR)/config/nekrs.mk
else

build_nekrs:
	echo "Skipping Nek build because ENABLE_NEK is not set to 'yes'"

endif

include            $(CARDINAL_DIR)/config/openmc.mk

# ======================================================================================
# Building app objects defined in app.mk
# ======================================================================================

# ADDITIONAL_LIBS are used for linking in app.mk
# CC_LINKER_SLFLAG is from petscvariables
ADDITIONAL_LIBS := \
	-L$(CARDINAL_DIR)/lib \
	-L$(OPENMC_LIBDIR) \
	-lopenmc \
	-lhdf5_hl \
	$(CC_LINKER_SLFLAG)$(CARDINAL_DIR)/lib \
	$(CC_LINKER_SLFLAG)$(OPENMC_LIBDIR)

ifeq ($(ENABLE_NEK), yes)
  ADDITIONAL_LIBS += -L$(NEKRS_LIBDIR) \
                     -lnekrs \
	                   -locca \
                     $(CC_LINKER_SLFLAG)$(NEKRS_LIBDIR)
endif

include            $(FRAMEWORK_DIR)/app.mk

# app_objects are defined in moose.mk and built according to the rules in build.mk
# We need to build these first so we get include dirs
$(app_objects): build_nekrs build_openmc
$(test_objects): build_nekrs build_openmc

CARDINAL_EXTERNAL_FLAGS := \
	-L$(CARDINAL_DIR)/lib \
	-L$(OPENMC_LIBDIR) \
	-L$(HDF5_LIBDIR) \
	-lopenmc \
	$(CC_LINKER_SLFLAG)$(CARDINAL_DIR)/lib \
	$(CC_LINKER_SLFLAG)$(OPENMC_LIBDIR) \
	$(CC_LINKER_SLFLAG)$(HDF5_LIBDIR) \
	$(BLASLAPACK_LIB) \
	$(PETSC_EXTERNAL_LIB_BASIC)

ifeq ($(ENABLE_NEK), yes)
  CARDINAL_EXTERNAL_FLAGS += -L$(NEKRS_LIBDIR) \
                             -lnekrs \
                             $(CC_LINKER_SLFLAG)$(NEKRS_LIBDIR)
endif

# EXTERNAL_FLAGS are for rules in app.mk
$(app_LIB): EXTERNAL_FLAGS := $(CARDINAL_EXTERNAL_FLAGS)
$(app_test_LIB): EXTERNAL_FLAGS := $(CARDINAL_EXTERNAL_FLAGS)
$(app_EXEC): EXTERNAL_FLAGS := $(CARDINAL_EXTERNAL_FLAGS)
