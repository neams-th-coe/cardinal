define n


endef

# First, we can find which submodules have been pulled in
MOOSE_CONTENT     := $(shell ls $(MOOSE_DIR) 2> /dev/null)
NEKRS_CONTENT     := $(shell ls $(NEKRS_DIR) 2> /dev/null)
OPENMC_CONTENT    := $(shell ls $(OPENMC_DIR) 2> /dev/null)
DAGMC_CONTENT     := $(shell ls $(DAGMC_DIR) 2> /dev/null)
MOAB_CONTENT      := $(shell ls $(MOAB_DIR) 2> /dev/null)
GRIFFIN_CONTENT   := $(shell ls $(GRIFFIN_DIR) 2> /dev/null)
BISON_CONTENT     := $(shell ls $(BISON_DIR) 2> /dev/null)
SAM_CONTENT       := $(shell ls $(SAM_DIR) 2> /dev/null)
SOCKEYE_CONTENT   := $(shell ls $(SOCKEYE_DIR) 2> /dev/null)

ifeq ($(THERMAL_HYDRAULICS), yes)
  THM_CONTENT     := true
endif

ifneq ($(BISON_CONTENT),)
  CONTACT            := yes
  MISC               := yes
  PHASE_FIELD        := yes
  XFEM               := yes
  SOLID_PROPERTIES   := yes
  LEVEL_SET          := yes
endif

SODIUM_CONTENT    := $(shell ls $(SODIUM_DIR) 2> /dev/null)
POTASSIUM_CONTENT := $(shell ls $(POTASSIUM_DIR) 2> /dev/null)
IAPWS95_CONTENT   := $(shell ls $(IAPWS95_DIR) 2> /dev/null)

ifeq ($(MOOSE_CONTENT),)
  $(error $n"MOOSE framework does not seem to be available. Make sure that either the submodule is checked out$nor that MOOSE_DIR points to a location with the MOOSE source.$n$nTo fetch the MOOSE submodule, use ./scripts/get-dependencies.sh")
endif

moose_status := $(shell git -C $(CONTRIB_DIR) submodule status 2>/dev/null | grep moose | cut -c1)
ifneq (,$(findstring +,$(moose_status)))
  $(warning $n"***WARNING***: Your MOOSE submodule is not pointing to the commit tied to Cardinal.$n                To fetch the paired commit, use ./scripts/get-dependencies.sh"$n)
endif

ifeq ($(ENABLE_NEK), yes)
  ifeq ($(NEKRS_CONTENT),)
    $(error $n"NekRS does not seem to be available, but ENABLE_NEK is enabled. Make sure that the submodule is checked out.$n$nTo fetch the NekRS submodule, use ./scripts/get-dependencies.sh")
  endif

  nek_status := $(shell git -C $(CONTRIB_DIR) submodule status 2>/dev/null | grep nekRS | cut -c1)
  ifneq (,$(findstring +,$(nek_status)))
    $(warning $n"***WARNING***: Your NekRS submodule is not pointing to the commit tied to Cardinal.$n                To fetch the paired commit, use ./scripts/get-dependencies.sh"$n)
  endif
endif

ifeq ($(ENABLE_OPENMC), yes)
  ifeq ($(OPENMC_CONTENT),)
    $(error $n"OpenMC does not seem to be available, but ENABLE_OPENMC is enabled. Make sure that the submodule is checked out.$n$nTo fetch the OpenMC submodule, use ./scripts/get-dependencies.sh")
  endif

  openmc_status := $(shell git -C $(CONTRIB_DIR) submodule status 2>/dev/null | grep openmc | cut -c1)
  ifneq (,$(findstring +,$(openmc_status)))
    $(warning $n"***WARNING***: Your OpenMC submodule is not pointing to the commit tied to Cardinal.$n                To fetch the paired commit, use ./scripts/get-dependencies.sh"$n)
  endif
endif

ifeq ($(ENABLE_DAGMC), yes)
  ifeq ($(DAGMC_CONTENT),)
    $(error $n"DagMC does not seem to be available, but ENABLE_DAGMC is enabled. Make sure that the submodule is checked out.$n$nTo fetch the DagMC submodule, use ./scripts/get-dependencies.sh")
  endif
  ifeq ($(MOAB_CONTENT),)
    $(error $n"Moab does not seem to be available, but ENABLE_DAGMC is enabled. Make sure that the submodule is checked out.$n$nTo fetch the Moab submodule, use ./scripts/get-dependencies.sh")
  endif

  DAGMC_status := $(shell git -C $(CONTRIB_DIR) submodule status 2>/dev/null | grep DAGMC | cut -c1)
  ifneq (,$(findstring +,$(DAGMC_status)))
    $(warning $n"***WARNING***: Your DagMC submodule is not pointing to the commit tied to Cardinal.$n                To fetch the paired commit, use ./scripts/get-dependencies.sh"$n)
  endif

  moab_status := $(shell git -C $(CONTRIB_DIR) submodule status 2>/dev/null | grep moab | cut -c1)
  ifneq (,$(findstring +,$(moab_status)))
    $(warning $n"***WARNING***: Your Moab submodule is not pointing to the commit tied to Cardinal.$n                To fetch the paired commit, use ./scripts/get-dependencies.sh"$n)
  endif
endif

# We can check that if it looks like we're going to build Sockeye, that
# all of its dependencies are there
ifneq ($(SOCKEYE_CONTENT),)
  ifeq ($(SODIUM_CONTENT),)
    $(error $n"Sodium dependency for Sockeye does not seem to be available. Make sure that either the submodule is checked out$nor that SODIUM_DIR points to a location with the sodium source.$n$nTo fetch the sodium submodule, use 'git submodule update --init contrib/sodium'")
  endif
  ifeq ($(POTASSIUM_CONTENT),)
    $(error $n"Potassium dependency for Sockeye does not seem to be available. Make sure that either the submodule is checked out$nor that POTASSIUM_DIR points to a location with the potassium source.$n$nTo fetch the potassium submodule, use 'git submodule update --init contrib/potassium'")
  endif
  ifeq ($(IAPWS95_CONTENT),)
    $(error $n"IAPWS95 dependency for Sockeye does not seem to be available. Make sure that either the submodule is checked out$nor that IAPWS95_DIR points to a location with the IAPWS95 source.$n$nTo fetch the IAPWS95 submodule, use 'git submodule update --init contrib/iapws95'")
  endif
endif

# We can check that if it looks like we're going to build Bison, that
# all of its dependencies are there
ifneq ($(BISON_CONTENT),)
  ifeq ($(IAPWS95_CONTENT),)
    $(error $n"IAPWS95 dependency for Bison does not seem to be available. Make sure that either the submodule is checked out$nor that IAPWS95_DIR points to a location with the IAPWS95 source.$n$nTo fetch the IAPWS95 submodule, use 'git submodule update --init contrib/iapws95'")
  endif
endif

# Cannot currently build with both SAM and Sockeye due to a conflict in THM.
# Someone might just build with THM (and not Sockeye), so we check both to be explicit.
ifneq ($(SOCKEYE_CONTENT),)
  ifneq ($(SAM_CONTENT),)
    $(error Cannot build Cardinal with both SAM and Sockeye due to a conflict)
  endif
endif
