[Mesh]
  type = FileMesh
  file = mesh.exo
  uniform_refine = 1
[]

[Variables]
  [temperature]
    initial_condition = 800.0
  []
[]

[AuxVariables]
  [source]
  []
[]

[AuxKernels]
  [source]
    type = FunctionAux
    variable = source
    function = heat_source
    block = '1'
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = temperature
  []
  [conduction]
    type = HeatConduction
    variable = temperature
  []
  [source]
    type = BodyForce
    variable = temperature
    function = heat_source
    block = '1'
  []
[]

[Functions]
  [heat_source]
    type = ParsedFunction
    value = 5000.0*cos(pi*z/0.5)*exp(x)
  []
[]

[BCs]
  [outer]
    type = DirichletBC
    variable = temperature
    boundary = 'outer'
    value = 500.0
  []
[]

[Materials]
  [k]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '2.5'
  []
[]

[Postprocessors]
  [min_temp_nek]
    type = NodalExtremeValue
    variable = temperature
    value_type = min
    block = 2
  []
  [max_temp_nek]
    type = NodalExtremeValue
    variable = temperature
    value_type = max
    block = 2
  []
  [min_temp_bison]
    type = NodalExtremeValue
    variable = temperature
    value_type = min
    block = 1
  []
  [max_temp_bison]
    type = NodalExtremeValue
    variable = temperature
    value_type = max
    block = 1
  []
  [max_interface]
    type = NodalExtremeValue
    variable = temperature
    value_type = max
    boundary = 'interface'
  []
  [min_interface]
    type = NodalExtremeValue
    variable = temperature
    value_type = min
    boundary = 'interface'
  []
  [flux_interface]
    type = SideDiffusiveFluxIntegral
    variable = temperature
    diffusivity = thermal_conductivity
    boundary = 'interface'
  []
[]

[Executioner]
  type = Transient
  dt = 0.5
  num_steps = 100
  nl_abs_tol = 1e-8
  steady_state_detection = true
  steady_state_tolerance = 1e-5
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  execute_on = 'final'
[]
