define n


endef

SPACE := $(subst ,, )

$(info $nCardinal is compiled with the following MOOSE modules)

ifeq ($(CHEMICAL_REACTIONS), yes)
  $(info $(SPACE) CHEMICAL_REACTIONS)
endif
ifeq ($(COMBINED), yes)
  $(info $(SPACE) COMBINED)
endif
ifeq ($(CONTACT), yes)
  $(info $(SPACE) CONTACT)
endif
ifeq ($(ELECTROMAGNETICS), yes)
  $(info $(SPACE) ELECTROMAGNETICS)
endif
ifeq ($(FLUID_PROPERTIES), yes)
  $(info $(SPACE) FLUID_PROPERTIES)
endif
ifeq ($(FSI), yes)
  $(info $(SPACE) FSI)
endif
ifeq ($(FUNCTIONAL_EXPANSION_TOOLS), yes)
  $(info $(SPACE) FUNCTIONAL_EXPANSION_TOOLS)
endif
ifeq ($(GEOMCHEMISTRY), yes)
  $(info $(SPACE) GEOCHEMISTRY)
endif
ifeq ($(HEAT_TRANSFER), yes)
  $(info $(SPACE) HEAT_TRANSFER)
endif
ifeq ($(LEVEL_SET), yes)
  $(info $(SPACE) LEVEL_SET)
endif
ifeq ($(MISC), yes)
  $(info $(SPACE) MISC)
endif
ifeq ($(NAVIER_STOKES), yes)
  $(info $(SPACE) NAVIER_STOKES)
endif
ifeq ($(OPTIMIZATION), yes)
  $(info $(SPACE) OPTIMIZATION)
endif
ifeq ($(PERIDYNAMICS), yes)
  $(info $(SPACE) PERIDYNAMICS)
endif
ifeq ($(PHASE_FIELD), yes)
  $(info $(SPACE) PHASE_FIELD)
endif
ifeq ($(POROUS_FLOW), yes)
  $(info $(SPACE) POROUS_FLOW)
endif
ifeq ($(RAY_TRACING), yes)
  $(info $(SPACE) RAY_TRACING)
endif
ifeq ($(REACTOR), yes)
  $(info $(SPACE) REACTOR)
endif
ifeq ($(RDG), yes)
  $(info $(SPACE) RDG)
endif
ifeq ($(RICHARDS), yes)
  $(info $(SPACE) RICHARDS)
endif
ifeq ($(SOLID_PROPERTIES), yes)
  $(info $(SPACE) SOLID_PROPERTIES)
endif
ifeq ($(STOCHASTIC_TOOLS), yes)
  $(info $(SPACE) STOCHASTIC_TOOLS)
endif
ifeq ($(SUBCHANNEL), yes)
  $(info $(SPACE) SUBCHANNEL)
endif
ifeq ($(TENSOR_MECHANICS), yes)
  $(info $(SPACE) TENSOR_MECHANICS)
endif
ifeq ($(THERMAL_HYDRAULICS), yes)
  $(info $(SPACE) THERMAL_HYDRAULICS)
endif
ifeq ($(XFEM), yes)
  $(info $(SPACE) XFEM)
endif
$(info $n)
