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

[ICs]
  [power]
    type = IntegralPreservingFunctionIC
    variable = power
    magnitude = 1e5
    function = power_shape
    integral = vol
  []
  [fluid_temp]
    type = BulkEnergyConservationIC
    variable = fluid_temp
    layered_integral = li
    magnitude = 1e5
    integral = vol

    mass_flowrate = 2.0
    cp = 1200
    inlet_T = 500.0
  []
[]

[UserObjects]
  [li]
    type = FunctionLayeredIntegral
    direction = z
    num_layers = 20
    cumulative = true
    function = power_shape
    execute_on = 'initial'
  []
[]

[Postprocessors]
  [vol]
    type = FunctionElementIntegral
    function = power_shape
    execute_on = 'initial'
  []
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
  num_steps = 1
[]

[Outputs]
  exodus = true
  hide = 'vol'
[]
