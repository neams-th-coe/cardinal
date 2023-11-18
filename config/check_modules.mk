define n


endef

$(info $nCardinal is compiled with the following MOOSE modules)

ifeq ($(CHEMICAL_REACTIONS), yes)
  $(info  CHEMICAL_REACTIONS)
endif
ifeq ($(COMBINED), yes)
  $(info  COMBINED)
endif
ifeq ($(CONTACT), yes)
  $(info  CONTACT)
endif
ifeq ($(ELECTROMAGNETICS), yes)
  $(info  ELECTROMAGNETICS)
endif
ifeq ($(FLUID_PROPERTIES), yes)
  $(info  FLUID_PROPERTIES)
endif
ifeq ($(FSI), yes)
  $(info  FSI)
endif
ifeq ($(FUNCTIONAL_EXPANSION_TOOLS), yes)
  $(info  FUNCTIONAL_EXPANSION_TOOLS)
endif
ifeq ($(GEOMCHEMISTRY), yes)
  $(info  GEOCHEMISTRY)
endif
ifeq ($(HEAT_TRANSFER), yes)
  $(info  HEAT_TRANSFER)
endif
ifeq ($(LEVEL_SET), yes)
  $(info  LEVEL_SET)
endif
ifeq ($(MISC), yes)
  $(info  MISC)
endif
ifeq ($(NAVIER_STOKES), yes)
  $(info  NAVIER_STOKES)
endif
ifeq ($(OPTIMIZATION), yes)
  $(info  OPTIMIZATION)
endif
ifeq ($(PERIDYNAMICS), yes)
  $(info  PERIDYNAMICS)
endif
ifeq ($(PHASE_FIELD), yes)
  $(info  PHASE_FIELD)
endif
ifeq ($(POROUS_FLOW), yes)
  $(info  POROUS_FLOW)
endif
ifeq ($(RAY_TRACING), yes)
  $(info  RAY_TRACING)
endif
ifeq ($(REACTOR), yes)
  $(info  REACTOR)
endif
ifeq ($(RDG), yes)
  $(info  RDG)
endif
ifeq ($(RICHARDS), yes)
  $(info  RICHARDS)
endif
ifeq ($(SOLID_PROPERTIES), yes)
  $(info  SOLID_PROPERTIES)
endif
ifeq ($(STOCHASTIC_TOOLS), yes)
  $(info  STOCHASTIC_TOOLS)
endif
ifeq ($(TENSOR_MECHANICS), yes)
  $(info  TENSOR_MECHANICS)
endif
ifeq ($(THERMAL_HYDRAULICS), yes)
  $(info  THERMAL_HYDRAULICS)
endif
ifeq ($(XFEM), yes)
  $(info  XFEM)
endif
$(info $n)
