[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 50
  ny = 50
  nz = 50
[]

[Variables]
  [temperature]
    initial_condition = 500.0
  []
[]

[AuxVariables]
  [source]
  []
[]

[Kernels]
  [conduction]
    type = HeatConduction
    variable = temperature
  []
  [source]
    type = CoupledForce
    variable = temperature
    v = source
  []
[]

[AuxKernels]
  [source]
    type = ParsedAux
    variable = source
    function = 'temperature*7'
    args = 'temperature'
    execute_on = 'linear'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = temperature
    boundary = 'left'
    value = 500.0
  []
  [right]
    type = DirichletBC
    variable = temperature
    boundary = 'right'
    value = 600.0
  []
  [top]
    type = DirichletBC
    variable = temperature
    boundary = 'top'
    value = 700.0
  []
  [front]
    type = DirichletBC
    variable = temperature
    boundary = 'front'
    value = 700.0
  []
[]

[Materials]
  [k]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '1.5'
  []
[]

[Postprocessors]
  [flux_left]
    type = SideFluxIntegral
    variable = temperature
    diffusivity = thermal_conductivity
    boundary = 'left'
  []
  [flux_right]
    type = SideFluxIntegral
    variable = temperature
    diffusivity = thermal_conductivity
    boundary = 'right'
  []
  [flux_top]
    type = SideFluxIntegral
    variable = temperature
    diffusivity = thermal_conductivity
    boundary = 'top'
  []
  [flux_front]
    type = SideFluxIntegral
    variable = temperature
    diffusivity = thermal_conductivity
    boundary = 'front'
  []
  [source]
    type = ElementIntegralVariablePostprocessor
    variable = source
  []
  [max_T]
    type = NodalExtremeValue
    variable = temperature
    value_type = max
  []
  [min_T]
    type = NodalExtremeValue
    variable = temperature
    value_type = min
  []
  [avg_T_volume]
    type = ElementAverageValue
    variable = temperature
  []
  [avg_T_back]
    type = SideAverageValue
    variable = temperature
    boundary = 'back'
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 10
  nl_abs_tol = 1e-8
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  execute_on = 'final'
[]

