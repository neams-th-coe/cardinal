[Mesh]
  [fuel]
    type = FileMeshGenerator
    file = krusty_fuel.e
  []
[]

[Variables]
  [T]
  []
[]

[AuxVariables]
  [power]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = T
  []
  [power]
    type = CoupledForce
    variable = T
    v = power
  []
[]

[BCs]
  [surface]
    type = DirichletBC
    variable = T
    boundary = 'heat_pipes'
    value = 800
  []
[]

[Materials]
  [k]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '0.1'
  []
[]

[Postprocessors]
  [tally_integral]
    type = ElementIntegralVariablePostprocessor
    variable = power
    execute_on = transfer
  []
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
[]
