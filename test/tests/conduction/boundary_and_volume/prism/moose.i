[Mesh]
  type = FileMesh
  file = prism.exo
  uniform_refine = 2
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
  [source1]
    type = ParsedAux
    variable = source
    coupled_variables = 'temperature'
    function = 'temperature+50'
    block = '1'
  []
  [source2]
    type = ParsedAux
    variable = source
    coupled_variables = 'temperature'
    function = '0.5*temperature+10'
    block = '2'
  []
[]

[Functions]
  [vol2_top]
    type = ParsedFunction
    expression = '500.0+500*exp(y)'
  []
  [vol1_top]
    type = ParsedFunction
    expression = '600.0+500*exp(y)'
  []
[]

[BCs]
  [left]
    type = FunctionDirichletBC
    variable = temperature
    boundary = 'vol2_top'
    function = 'vol2_top'
  []
  [right]
    type = FunctionDirichletBC
    variable = temperature
    boundary = 'vol1_top'
    function = 'vol1_top'
  []
[]

[Materials]
  [k]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '10.5'
    block = '1'
  []
  [k2]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '5.0'
    block = '2'
  []
[]

[Postprocessors]
  [max_T_int]
    type = NodalExtremeValue
    variable = temperature
    value_type = max
    boundary = '2'
  []
  [min_T_int]
    type = NodalExtremeValue
    variable = temperature
    value_type = min
    boundary = '2'
  []
  [max_T_nek]
    type = NodalExtremeValue
    variable = temperature
    value_type = max
    block = '1'
  []
  [min_T_nek]
    type = NodalExtremeValue
    variable = temperature
    value_type = min
    block = '1'
  []
  [avg_T_nek]
    type = ElementAverageValue
    variable = temperature
    block = '1'
  []
  [max_T_bison]
    type = NodalExtremeValue
    variable = temperature
    value_type = max
    block = '2'
  []
  [min_T_bison]
    type = NodalExtremeValue
    variable = temperature
    value_type = min
    block = '2'
  []
  [avg_T_bison]
    type = ElementAverageValue
    variable = temperature
    block = '2'
  []
  [pt_nek]
    type = PointValue
    point = '0 0 0'
    variable = temperature
  []
  [pt_bison]
    type = PointValue
    point = '0 0 -1'
    variable = temperature
  []
  [nek_min_1]
    type = NodalExtremeValue
    variable = temperature
    boundary = 'vol1_face1'
    value_type = min
  []
  [nek_min_2]
    type = NodalExtremeValue
    variable = temperature
    boundary = 'vol1_face2'
    value_type = min
  []
  [nek_min_3]
    type = NodalExtremeValue
    variable = temperature
    boundary = 'vol1_face3'
    value_type = min
  []
  [nek_min_4]
    type = NodalExtremeValue
    variable = temperature
    boundary = 'vol1_face4'
    value_type = min
  []
  [nek_max_1]
    type = NodalExtremeValue
    variable = temperature
    boundary = 'vol1_face1'
  []
  [nek_max_2]
    type = NodalExtremeValue
    variable = temperature
    boundary = 'vol1_face2'
  []
  [nek_max_3]
    type = NodalExtremeValue
    variable = temperature
    boundary = 'vol1_face3'
  []
  [nek_max_4]
    type = NodalExtremeValue
    variable = temperature
    boundary = 'vol1_face4'
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
[]

