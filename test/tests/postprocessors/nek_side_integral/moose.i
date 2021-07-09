[Mesh]
  type = FileMesh
  file = ../meshes/pyramid.exo
[]

[AuxVariables]
  [temp_test]
  []
  [pressure_test]
  []
  [velocity_test]
  []
[]

[ICs]
  [temp_test]
    type = FunctionIC
    variable = temp_test
    function = temp
  []
  [pressure_test]
    type = FunctionIC
    variable = pressure_test
    function = pressure
  []
  [velocity_test]
    type = FunctionIC
    variable = velocity_test
    function = velocity
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
    value = 'exp(x)+sin(y)+x*y*z'
  []
  [pressure]
    type = ParsedFunction
    value = 'x+y+z+sin(x*y)'
  []
  [velocity]
     type = ParsedFunction
     value = 'sqrt(sin(x)*sin(x)+(y+1)*(y+1)+exp(x*y*z)*exp(x*y*z))'
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
  [area_side1]
    type = AreaPostprocessor
    boundary = '1'
  []
  [area_side2]
    type = AreaPostprocessor
    boundary = '2'
  []
  [area_side3]
    type = AreaPostprocessor
    boundary = '3'
  []
  [area_side4]
    type = AreaPostprocessor
    boundary = '4'
  []
  [area_side5]
    type = AreaPostprocessor
    boundary = '5'
  []
  [area_side6]
    type = AreaPostprocessor
    boundary = '6'
  []
  [area_side7]
    type = AreaPostprocessor
    boundary = '7'
  []
  [area_side8]
    type = AreaPostprocessor
    boundary = '8'
  []
  [temp_side1]
    type = SideIntegralVariablePostprocessor
    variable = temp_test
    boundary = '1'
  []
  [temp_side2]
    type = SideIntegralVariablePostprocessor
    variable = temp_test
    boundary = '2'
  []
  [temp_side3]
    type = SideIntegralVariablePostprocessor
    variable = temp_test
    boundary = '3'
  []
  [temp_side4]
    type = SideIntegralVariablePostprocessor
    variable = temp_test
    boundary = '4'
  []
  [temp_side5]
    type = SideIntegralVariablePostprocessor
    variable = temp_test
    boundary = '5'
  []
  [temp_side6]
    type = SideIntegralVariablePostprocessor
    variable = temp_test
    boundary = '6'
  []
  [temp_side7]
    type = SideIntegralVariablePostprocessor
    variable = temp_test
    boundary = '7'
  []
  [temp_side8]
    type = SideIntegralVariablePostprocessor
    variable = temp_test
    boundary = '8'
  []
  [pressure_side1]
    type = SideIntegralVariablePostprocessor
    variable = pressure_test
    boundary = '1'
  []
  [pressure_side2]
    type = SideIntegralVariablePostprocessor
    variable = pressure_test
    boundary = '2'
  []
  [pressure_side3]
    type = SideIntegralVariablePostprocessor
    variable = pressure_test
    boundary = '3'
  []
  [pressure_side4]
    type = SideIntegralVariablePostprocessor
    variable = pressure_test
    boundary = '4'
  []
  [pressure_side5]
    type = SideIntegralVariablePostprocessor
    variable = pressure_test
    boundary = '5'
  []
  [pressure_side6]
    type = SideIntegralVariablePostprocessor
    variable = pressure_test
    boundary = '6'
  []
  [pressure_side7]
    type = SideIntegralVariablePostprocessor
    variable = pressure_test
    boundary = '7'
  []
  [pressure_side8]
    type = SideIntegralVariablePostprocessor
    variable = pressure_test
    boundary = '8'
  []
  [velocity_avg1]
    type = SideIntegralVariablePostprocessor
    variable = velocity_test
    boundary = '1'
  []
  [velocity_avg2]
    type = SideIntegralVariablePostprocessor
    variable = velocity_test
    boundary = '2'
  []
  [velocity_avg3]
    type = SideIntegralVariablePostprocessor
    variable = velocity_test
    boundary = '3'
  []
  [velocity_avg4]
    type = SideIntegralVariablePostprocessor
    variable = velocity_test
    boundary = '4'
  []
  [velocity_avg5]
    type = SideIntegralVariablePostprocessor
    variable = velocity_test
    boundary = '5'
  []
  [velocity_avg6]
    type = SideIntegralVariablePostprocessor
    variable = velocity_test
    boundary = '6'
  []
  [velocity_avg7]
    type = SideIntegralVariablePostprocessor
    variable = velocity_test
    boundary = '7'
  []
  [velocity_avg8]
    type = SideIntegralVariablePostprocessor
    variable = velocity_test
    boundary = '8'
  []
[]
