define n


endef

# Set default values for all third party dependencies
NEKRS_DIR           ?= $(CONTRIB_DIR)/nekRS
OPENMC_DIR          ?= $(CONTRIB_DIR)/openmc
DAGMC_DIR           ?= $(CONTRIB_DIR)/DAGMC
DOUBLEDOWN_DIR      ?= $(CONTRIB_DIR)/double-down
EMBREE_DIR          ?= $(CONTRIB_DIR)/embree
MOAB_DIR            ?= $(CONTRIB_DIR)/moab
GRIFFIN_DIR         ?= $(CONTRIB_DIR)/griffin
BISON_DIR           ?= $(CONTRIB_DIR)/bison
SAM_DIR             ?= $(CONTRIB_DIR)/SAM
SOCKEYE_DIR         ?= $(CONTRIB_DIR)/sockeye
SODIUM_DIR          ?= $(CONTRIB_DIR)/sodium
POTASSIUM_DIR       ?= $(CONTRIB_DIR)/potassium
IAPWS95_DIR         ?= $(CONTRIB_DIR)/iapws95

# Then, we can find which optional dependencies we have been pulled in
# by seeing if those directories are empty or not
MOOSE_CONTENT      := $(shell ls $(MOOSE_DIR) 2> /dev/null)
NEKRS_CONTENT      := $(shell ls $(NEKRS_DIR) 2> /dev/null)
OPENMC_CONTENT     := $(shell ls $(OPENMC_DIR) 2> /dev/null)
DAGMC_CONTENT      := $(shell ls $(DAGMC_DIR) 2> /dev/null)
DOUBLEDOWN_CONTENT := $(shell ls $(DOUBLEDOWN_DIR) 2> /dev/null)
EMBREE_CONTENT     := $(shell ls $(EMBREE_DIR) 2> /dev/null)
MOAB_CONTENT       := $(shell ls $(MOAB_DIR) 2> /dev/null)
GRIFFIN_CONTENT    := $(shell ls $(GRIFFIN_DIR) 2> /dev/null)
BISON_CONTENT      := $(shell ls $(BISON_DIR) 2> /dev/null)
SAM_CONTENT        := $(shell ls $(SAM_DIR) 2> /dev/null)
SOCKEYE_CONTENT    := $(shell ls $(SOCKEYE_DIR) 2> /dev/null)
SODIUM_CONTENT     := $(shell ls $(SODIUM_DIR) 2> /dev/null)
POTASSIUM_CONTENT  := $(shell ls $(POTASSIUM_DIR) 2> /dev/null)
IAPWS95_CONTENT    := $(shell ls $(IAPWS95_DIR) 2> /dev/null)

# convert ENABLE_NEK, ENABLE_OPENMC, and ENABLE_DAGMC to consistent truthy value
ifeq ($(ENABLE_OPENMC),$(filter $(ENABLE_OPENMC), true yes on 1 TRUE YES ON))
  ENABLE_OPENMC := yes
endif
ifeq ($(ENABLE_NEK),$(filter $(ENABLE_NEK), true yes on 1 TRUE YES ON))
  ENABLE_NEK := yes
endif
ifeq ($(ENABLE_DAGMC),$(filter $(ENABLE_DAGMC), true yes on 1 TRUE YES ON))
  ENABLE_DAGMC := yes
endif

ifeq ($(ENABLE_OPENMC), yes)
  # HDF5 is only needed to be linked if using OpenMC
  $(info Cardinal is using HDF5 from      $(HDF5_ROOT))
else
  ifeq ($(ENABLE_DAGMC), yes)
    $(info Ignoring ENABLE_DAGMC because OpenMC is not enabled.)
    ENABLE_DAGMC := no
  endif
endif

ifeq ($(MOOSE_CONTENT),)
  $(error $n"MOOSE framework does not seem to be available. Make sure that either the submodule is checked out$nor that MOOSE_DIR points to a location with the MOOSE source.$n$nTo fetch the MOOSE submodule, use ./scripts/get-dependencies.sh")
else
  $(info Cardinal is using MOOSE from           $(MOOSE_DIR))
endif

moose_status := $(shell git -C $(CONTRIB_DIR) submodule status 2>/dev/null | grep moose | cut -c1)
ifneq (,$(findstring +,$(moose_status)))
  $(warning $n"***WARNING***: Your MOOSE submodule is not pointing to the commit tied to Cardinal.$n                To fetch the paired commit, use ./scripts/get-dependencies.sh"$n)
endif

ifeq ($(ENABLE_NEK), yes)
  ifeq ($(NEKRS_CONTENT),)
    $(error $n"NekRS does not seem to be available, but ENABLE_NEK is enabled. Make sure that the submodule is checked out.$n$nTo fetch the NekRS submodule, use ./scripts/get-dependencies.sh")
  else
    $(info Cardinal is using NekRS from           $(NEKRS_DIR))
  endif

  nek_status := $(shell git -C $(CONTRIB_DIR) submodule status 2>/dev/null | grep nekRS | cut -c1)
  ifneq (,$(findstring +,$(nek_status)))
    $(warning $n"***WARNING***: Your NekRS submodule is not pointing to the commit tied to Cardinal.$n                To fetch the paired commit, use ./scripts/get-dependencies.sh"$n)
  endif
endif

ifeq ($(ENABLE_OPENMC), yes)
  ifeq ($(OPENMC_CONTENT),)
    $(error $n"OpenMC does not seem to be available, but ENABLE_OPENMC is enabled. Make sure that the submodule is checked out.$n$nTo fetch the OpenMC submodule, use ./scripts/get-dependencies.sh")
  else
    $(info Cardinal is using OpenMC from          $(OPENMC_DIR))
  endif

  openmc_status := $(shell git -C $(CONTRIB_DIR) submodule status 2>/dev/null | grep openmc | cut -c1)
  ifneq (,$(findstring +,$(openmc_status)))
    $(warning $n"***WARNING***: Your OpenMC submodule is not pointing to the commit tied to Cardinal.$n                To fetch the paired commit, use ./scripts/get-dependencies.sh"$n)
  endif
endif

ifeq ($(ENABLE_DAGMC), yes)
  ifeq ($(DAGMC_CONTENT),)
    $(error $n"DagMC does not seem to be available, but ENABLE_DAGMC is enabled. Make sure that the submodule is checked out.$n$nTo fetch the DagMC submodule, use ./scripts/get-dependencies.sh")
  else
    $(info Cardinal is using DAGMC from           $(DAGMC_DIR))
  endif
	ifeq ($(DOUBLEDOWN_CONTENT),)
    $(error $n"Double-Down does not seem to be available, but ENABLE_DAGMC is enabled. Make sure that the submodule is checked out.$n$nTo fetch the Double-Down submodule, use ./scripts/get-dependencies.sh")
  else
    $(info Cardinal is using Double-Down from     $(DOUBLEDOWN_DIR))
  endif
	ifeq ($(EMBREE_CONTENT),)
    $(error $n"Embree does not seem to be available, but ENABLE_DAGMC is enabled. Make sure that the submodule is checked out.$n$nTo fetch the Embree submodule, use ./scripts/get-dependencies.sh")
  else
    $(info Cardinal is using Embree from          $(EMBREE_DIR))
  endif
  ifeq ($(MOAB_CONTENT),)
    $(error $n"Moab does not seem to be available, but ENABLE_DAGMC is enabled. Make sure that the submodule is checked out.$n$nTo fetch the Moab submodule, use ./scripts/get-dependencies.sh")
  else
    $(info Cardinal is using Moab from            $(MOAB_DIR))
  endif

  DAGMC_status := $(shell git -C $(CONTRIB_DIR) submodule status 2>/dev/null | grep DAGMC | cut -c1)
  ifneq (,$(findstring +,$(DAGMC_status)))
    $(warning $n"***WARNING***: Your DagMC submodule is not pointing to the commit tied to Cardinal.$n                To fetch the paired commit, use ./scripts/get-dependencies.sh"$n)
  endif

	EMBREE_status := $(shell git -C $(CONTRIB_DIR) submodule status 2>/dev/null | grep embree | cut -c1)
  ifneq (,$(findstring +,$(EMBREE_status)))
    $(warning $n"***WARNING***: Your Embree submodule is not pointing to the commit tied to Cardinal.$n                To fetch the paired commit, use ./scripts/get-dependencies.sh"$n)
  endif

	DOUBLEDOWN_status := $(shell git -C $(CONTRIB_DIR) submodule status 2>/dev/null | grep double-down | cut -c1)
  ifneq (,$(findstring +,$(DOUBLEDOWN_status)))
    $(warning $n"***WARNING***: Your Double-Down submodule is not pointing to the commit tied to Cardinal.$n                To fetch the paired commit, use ./scripts/get-dependencies.sh"$n)
  endif

  moab_status := $(shell git -C $(CONTRIB_DIR) submodule status 2>/dev/null | grep moab | cut -c1)
  ifneq (,$(findstring +,$(moab_status)))
    $(warning $n"***WARNING***: Your Moab submodule is not pointing to the commit tied to Cardinal.$n                To fetch the paired commit, use ./scripts/get-dependencies.sh"$n)
  endif
endif

ifneq ($(SAM_CONTENT),)
  $(info Cardinal is using SAM from       $(SAM_DIR))
endif

ifneq ($(BISON_CONTENT),)
  # get the modules that Bison requires; we list all of those from the Bison
  # makefile, regardless of what value is set for these in the Cardinal makefile
  $(info Cardinal is using BISON from     $(BISON_DIR))
  CONTACT            := yes
  FLUID_PROPERTIES   := yes
  HEAT_TRANSFER      := yes
  LEVEL_SET          := yes
  MISC               := yes
  PHASE_FIELD        := yes
  SOLID_PROPERTIES   := yes
  STOCHASTIC_TOOLS   := yes
  SOLID_MECHANICS    := yes
  THERMAL_HYDRAULICS := yes
  XFEM               := yes

  # We can check that if it looks like we're going to build Bison, that
  # all of its dependencies are there
  ifeq ($(IAPWS95_CONTENT),)
    $(error $n"IAPWS95 dependency for Bison does not seem to be available. Make sure that either the submodule is checked out$nor that IAPWS95_DIR points to a location with the IAPWS95 source.$n$nTo fetch the IAPWS95 submodule, use 'git submodule update --init contrib/iapws95'")
  else
    ifeq ($(SOCKEYE_CONTENT),)
      # don't print location info for IAPWS twice, if we are building with both
      # Sockeye and Bison (since both require IAPWS)
      $(info Cardinal is using water from     $(IAPWS95_DIR))
    endif
  endif
endif

ifneq ($(GRIFFIN_CONTENT),)
  # get the modules that Griffin requires; we list all of those from the Griffin
  # makefile, regardless of what value is set for these in the Cardinal makefile
  $(info Cardinal is using Griffin from   $(GRIFFIN_DIR))
  HEAT_TRANSFER      := yes
  PHASE_FIELD        := yes
  MISC               := yes
  NAVIER_STOKES      := yes
  RAY_TRACING        := yes
  SOLID_PROPERTIES   := yes
  STOCHASTIC_TOOLS   := yes
  REACTOR            := yes
  THERMAL_HYDRAULICS := yes
endif

ifneq ($(SOCKEYE_CONTENT),)
  # get the modules that Sockeye requires; we list all of those from the Sockeye
  # makefile, regardless of what value is set for these in the Cardinal makefile
  $(info Cardinal is using Sockeye from   $(SOCKEYE_DIR))
  FLUID_PROPERTIES   := yes
  HEAT_TRANSFER      := yes
  MISC               := yes
  NAVIER_STOKES      := yes
  RAY_TRACING        := yes
  RDG                := yes
  REACTOR            := yes
  SOLID_PROPERTIES   := yes
  STOCHASTIC_TOOLS   := yes
  THERMAL_HYDRAULICS := yes

  # We can check that if it looks like we're going to build Sockeye, that
  # all of its dependencies are there
  ifeq ($(SODIUM_CONTENT),)
    $(error $n"Sodium dependency for Sockeye does not seem to be available. Make sure that either the submodule is checked out$nor that SODIUM_DIR points to a location with the sodium source.$n$nTo fetch the sodium submodule, use 'git submodule update --init contrib/sodium'")
  else
    $(info Cardinal is using sodium from    $(SODIUM_DIR))
  endif
  ifeq ($(POTASSIUM_CONTENT),)
    $(error $n"Potassium dependency for Sockeye does not seem to be available. Make sure that either the submodule is checked out$nor that POTASSIUM_DIR points to a location with the potassium source.$n$nTo fetch the potassium submodule, use 'git submodule update --init contrib/potassium'")
  else
    $(info Cardinal is using potassium from $(POTASSIUM_DIR))
  endif
  ifeq ($(IAPWS95_CONTENT),)
    $(error $n"IAPWS95 dependency for Sockeye does not seem to be available. Make sure that either the submodule is checked out$nor that IAPWS95_DIR points to a location with the IAPWS95 source.$n$nTo fetch the IAPWS95 submodule, use 'git submodule update --init contrib/iapws95'")
  else
    $(info Cardinal is using water from     $(IAPWS95_DIR))
  endif
endif

# Cannot currently build with both SAM and Sockeye due to a conflict in THM.
# Someone might just build with THM (and not Sockeye), so we check both to be explicit.
ifneq ($(SOCKEYE_CONTENT),)
  ifneq ($(SAM_CONTENT),)
    $(error Cannot build Cardinal with both SAM and Sockeye due to a conflict)
  endif
endif
