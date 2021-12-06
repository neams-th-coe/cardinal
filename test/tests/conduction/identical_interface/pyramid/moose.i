[Mesh]
  type = FileMesh
  file = mesh.exo
  uniform_refine = 1
[]

[Variables]
  [temperature]
    initial_condition = 1000.0
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
[]

[Functions]
  [top]
    type = ParsedFunction
    value = -1000*exp(x)*exp(y)*exp(z)
  []
  [bottom]
    type = ParsedFunction
    value = 1500+2000*x
  []
[]

[BCs]
  [vol1_top]
    type = FunctionDirichletBC
    variable = temperature
    boundary = 'vol1_top'
    function = top
  []
  [vol2_top]
    type = FunctionDirichletBC
    variable = temperature
    boundary = 'vol2_top'
    function = bottom
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
    block = 1
  []
  [max_temp_nek]
    type = NodalExtremeValue
    variable = temperature
    value_type = max
    block = 1
  []
  [min_temp_bison]
    type = NodalExtremeValue
    variable = temperature
    value_type = min
    block = 2
  []
  [max_temp_bison]
    type = NodalExtremeValue
    variable = temperature
    value_type = max
    block = 2
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
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 100
  nl_abs_tol = 1e-8
  steady_state_detection = true
  steady_state_tolerance = 1e-5
[]

[Outputs]
  exodus = true
  file_base = newo
  print_linear_residuals = false
  execute_on = 'final'
[]

