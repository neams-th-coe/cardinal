[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 30
  ny = 30
  nz = 30
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
    value = 650.0
  []
  [front]
    type = DirichletBC
    variable = temperature
    boundary = 'front'
    value = 650.0
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
  [flux_out]
    type = SideDiffusiveFluxIntegral
    variable = temperature
    boundary = 'front back top bottom left right'
    diffusivity = thermal_conductivity
  []
[]

[Executioner]
  type = Steady
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  execute_on = 'final'
[]

