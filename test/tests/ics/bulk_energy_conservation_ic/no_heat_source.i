[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 5
  ny = 5
  nz = 40
  xmax = 1.5
  ymax = 1.7
  zmax = 1.9
  xmin = 0.0
  ymin = 0.0
  zmin = 0.0
[]

[Problem]
  type = FEProblem
  solve = false
[]

[AuxVariables]
  [power]
    family = MONOMIAL
    order = CONSTANT
  []
  [fluid_temp]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Functions]
  [power_shape]
    type = ParsedFunction
    expression = 'sin(pi * z / 1.9)'
  []
[]

[Cardinal]
  [ICs]
    [BulkEnergyConservation]
      variable = fluid_temp
      flow_direction = z
      num_layers = 20

      mass_flowrate = 2.0
      cp = 1200
      inlet_T = 500.0
    []
  []
[]

[Postprocessors]
  [integrated_power] # should equal 550
    type = ElementIntegralVariablePostprocessor
    variable = power
  []
  [max_Tf]
    type = ElementExtremeValue
    variable = fluid_temp
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  exodus = true
  file_base = sinusoidal_z_out
[]
