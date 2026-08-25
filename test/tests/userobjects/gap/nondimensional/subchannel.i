[Mesh]
  type = HexagonalSubchannelGapMesh
  bundle_pitch = 0.02583914354890463
  pin_pitch = 0.0089656996
  pin_diameter = 7.646e-3
  n_rings = 2
  n_axial = 6
  height = 0.008
[]

[Problem]
  solve = false
  type = FEProblem
[]

[AuxVariables]
  [avg_T]
    family = MONOMIAL
    order = CONSTANT
  []
  [avg_p]
    family = MONOMIAL
    order = CONSTANT
  []
  [avg_v]
    family = MONOMIAL
    order = CONSTANT
  []
  [temp]
    family = MONOMIAL
    order = CONSTANT
  []
  [velocity]
    family = MONOMIAL
    order = CONSTANT
  []
  [diff_T]
    family = MONOMIAL
    order = CONSTANT
  []
  [diff_v]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [diff_T]
    type = ParsedAux
    variable= diff_T
    expression = 'abs(temp-avg_T)/abs(temp)'
    coupled_variables = 'temp avg_T'
  []
  [diff_v]
    type = ParsedAux
    variable= diff_v
    expression = 'abs(velocity-avg_v)/abs(velocity)'
    coupled_variables = 'velocity avg_v'
  []
[]

[Executioner]
  type = Transient
[]

[Postprocessors]
  [max_T]
    type = ElementExtremeValue
    variable = temp
  []
  [min_T]
    type = ElementExtremeValue
    variable = temp
    value_type = min
  []
  [max_v]
    type = ElementExtremeValue
    variable = velocity
  []
  [min_v]
    type = ElementExtremeValue
    variable = velocity
    value_type = min
  []
  [max_T_diff]
    type = ElementExtremeValue
    variable = diff_T
  []
  [max_v_diff]
    type = ElementExtremeValue
    variable = diff_v
  []
  [fraction_T]
    type = ParsedPostprocessor
    expression = 'max_T_diff/(max_T-min_T)'
    pp_names = 'max_T_diff max_T min_T'
  []
  [pass_T]
    type = ParsedPostprocessor
    expression = 'if (fraction_T < 0.2, 1, 0)'
    pp_names = 'fraction_T'
  []
  [fraction_v]
    type = ParsedPostprocessor
    expression = 'max_v_diff/(max_v-min_v)'
    pp_names = 'max_v_diff max_v min_v'
  []
  [pass_v]
    type = ParsedPostprocessor
    expression = 'if (fraction_v < 0.2, 1, 0)'
    pp_names = 'fraction_v'
  []
[]

[Outputs]
  exodus = true
  csv = true
  hide = 'max_T_diff max_v_diff fraction_T fraction_v max_T min_T max_v min_v'
[]
