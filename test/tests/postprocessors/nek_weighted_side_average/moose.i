[Mesh]
  type = FileMesh
  file = ../meshes/brick.exo
[]

[AuxVariables]
  [temp_test]
  []
  [rho]
    initial_condition = 834.5
  []
  [vel_x]
  []
  [vel_y]
  []
  [vel_z]
  []
  [weighted_temp_xp]
  []
  [weighted_temp_xm]
  []
  [weighted_temp_yp]
  []
  [weighted_temp_ym]
  []
  [weighted_temp_zp]
  []
  [weighted_temp_zm]
  []
  [mdot_xp]
  []
  [mdot_xm]
  []
  [mdot_yp]
  []
  [mdot_ym]
  []
  [mdot_zp]
  []
  [mdot_zm]
  []
[]

[ICs]
  [temp_test]
    type = FunctionIC
    variable = temp_test
    function = temp
  []
  [vel_x]
    type = FunctionIC
    variable = vel_x
    function = vel_x
  []
  [vel_y]
    type = FunctionIC
    variable = vel_y
    function = vel_y
  []
  [vel_z]
    type = FunctionIC
    variable = vel_z
    function = vel_z
  []
[]

[AuxKernels]
  [mdot_xp]
    type = ParsedAux
    variable = mdot_xp
    function = '834.5*vel_x'
    coupled_variables='vel_x'
  []
  [mdot_xm]
    type = ParsedAux
    variable = mdot_xm
    function = '-834.5*vel_x'
    coupled_variables='vel_x'
  []
  [mdot_yp]
    type = ParsedAux
    variable = mdot_yp
    function = '834.5*vel_y'
    coupled_variables='vel_y'
  []
  [mdot_ym]
    type = ParsedAux
    variable = mdot_ym
    function = '-834.5*vel_y'
    coupled_variables='vel_y'
  []
  [mdot_zp]
    type = ParsedAux
    variable = mdot_zp
    function = '834.5*vel_z'
    coupled_variables='vel_z'
  []
  [mdot_zm]
    type = ParsedAux
    variable = mdot_zm
    function = '-834.5*vel_z'
    coupled_variables='vel_z'
  []

  [weighted_temp_xp]
    type = ParsedAux
    variable = weighted_temp_xp
    function = '834.5*vel_x*temp_test'
    coupled_variables='vel_x temp_test'
  []
  [weighted_temp_xm]
    type = ParsedAux
    variable = weighted_temp_xm
    function = '-834.5*vel_x*temp_test'
    coupled_variables='vel_x temp_test'
  []
  [weighted_temp_yp]
    type = ParsedAux
    variable = weighted_temp_yp
    function = '834.5*vel_y*temp_test'
    coupled_variables='vel_y temp_test'
  []
  [weighted_temp_ym]
    type = ParsedAux
    variable = weighted_temp_ym
    function = '-834.5*vel_y*temp_test'
    coupled_variables='vel_y temp_test'
  []
  [weighted_temp_zp]
    type = ParsedAux
    variable = weighted_temp_zp
    function = '834.5*vel_z*temp_test'
    coupled_variables='vel_z temp_test'
  []
  [weighted_temp_zm]
    type = ParsedAux
    variable = weighted_temp_zm
    function = '-834.5*vel_z*temp_test'
    coupled_variables='vel_z temp_test'
  []
[]

[Variables]
  [dummy]
  []
[]

[Kernels]
  [dummy]
    type = Diffusion
    variable = dummy
  []
[]

[BCs]
  [fixed]
    type = DirichletBC
    variable = dummy
    value = 1.0
    boundary = '1'
  []
[]

[Functions]
  [temp]
    type = ParsedFunction
    expression = 'exp(x)+sin(y)+x*y*z'
  []
  [vel_x]
    type = ParsedFunction
    expression = '1+x*y*z'
  []
  [vel_y]
    type = ParsedFunction
    expression = '2+y*exp(z)'
  []
  [vel_z]
    type = ParsedFunction
    expression = '3+z*x'
  []
[]

[Executioner]
  type = Transient
  dt = 5e-4
  num_steps = 1
[]

[Outputs]
  csv = true
  execute_on = 'final'
[]

[Postprocessors]
  [weighted_T_side1]
    type = SideIntegralVariablePostprocessor
    variable = weighted_temp_ym
    boundary = '1'
  []
  [weighted_T_side2]
    type = SideIntegralVariablePostprocessor
    variable = weighted_temp_yp
    boundary = '2'
  []
  [weighted_T_side3]
    type = SideIntegralVariablePostprocessor
    variable = weighted_temp_xm
    boundary = '3'
  []
  [weighted_T_side4]
    type = SideIntegralVariablePostprocessor
    variable = weighted_temp_xp
    boundary = '4'
  []
  [weighted_T_side5]
    type = SideIntegralVariablePostprocessor
    variable = weighted_temp_zp
    boundary = '5'
  []
  [weighted_T_side6]
    type = SideIntegralVariablePostprocessor
    variable = weighted_temp_zm
    boundary = '6'
  []
  [mdot_side1]
    type = SideIntegralVariablePostprocessor
    variable = mdot_ym
    boundary = '1'
  []
  [mdot_side2]
    type = SideIntegralVariablePostprocessor
    variable = mdot_yp
    boundary = '2'
  []
  [mdot_side3]
    type = SideIntegralVariablePostprocessor
    variable = mdot_xm
    boundary = '3'
  []
  [mdot_side4]
    type = SideIntegralVariablePostprocessor
    variable = mdot_xp
    boundary = '4'
  []
  [mdot_side5]
    type = SideIntegralVariablePostprocessor
    variable = mdot_zp
    boundary = '5'
  []
  [mdot_side6]
    type = SideIntegralVariablePostprocessor
    variable = mdot_zm
    boundary = '6'
  []
[]
