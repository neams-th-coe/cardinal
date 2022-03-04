define n


endef

ifeq ($(MOOSE_CONTENT),)
  $(error $n"MOOSE framework does not seem to be available. Make sure that either the submodule is checked out$nor that MOOSE_DIR points to a location with the MOOSE source.$n$nTo fetch the MOOSE submodule, use ./scripts/get-dependencies.sh")
endif

ifeq ($(ENABLE_NEK), yes)
  ifeq ($(NEKRS_CONTENT),)
    $(error $n"NekRS does not seem to be available, but ENABLE_NEK is set to 'yes'. Make sure that the submodule is checked out.$n$nTo fetch the NekRS submodule, use ./scripts/get-dependencies.sh")
  endif
endif

ifeq ($(ENABLE_OPENMC), yes)
  ifeq ($(OPENMC_CONTENT),)
    $(error $n"OpenMC does not seem to be available, but ENABLE_OPENMC is set to 'yes'. Make sure that the submodule is checked out.$n$nTo fetch the OpenMC submodule, use ./scripts-get-dependencies.sh")
  endif
endif

# We can check that if it looks like we're going to build Sockeye, that
# all of its dependencies are there
ifneq ($(SOCKEYE_CONTENT),)
  ifeq ($(THM_CONTENT),)
    $(error $n"Thermal Hydraulics Module dependency for Sockeye does not seem to be available.$n$nMake sure that 'THERMAL_HYDRAULICS := yes' is set in Cardinal's Makefile.")
  endif
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

# Cannot currently build with both SAM and Sockeye due to a conflict in THM.
# Someone might just build with THM (and not Sockeye), so we check both to be explicit.
ifneq ($(SOCKEYE_CONTENT),)
  ifneq ($(SAM_CONTENT),)
    $(error Cannot build Cardinal with both SAM and Sockeye due to a conflict)
  endif
endif

ifneq ($(THM_CONTENT),)
  ifneq ($(SAM_CONTENT),)
    $(error Cannot build Cardinal with both SAM and the Thermal Hydraulic Module due to a conflict. To build with SAM, you must set 'THERMAL_HYDRAULICS := no' in Cardinal's Makefile)
  endif
endif
