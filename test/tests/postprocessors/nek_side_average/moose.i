[Mesh]
  type = FileMesh
  file = ../meshes/pyramid.exo
[]

[AuxVariables]
  [temp_test]
  []
  [pressure_test]
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
    value = 'exp(x)+exp(y)+exp(z)'
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
  [temp_avg1]
    type = SideAverageValue
    variable = temp_test
    boundary = '1'
  []
  [temp_avg2]
    type = SideAverageValue
    variable = temp_test
    boundary = '2'
  []
  [temp_avg3]
    type = SideAverageValue
    variable = temp_test
    boundary = '3'
  []
  [temp_avg4]
    type = SideAverageValue
    variable = temp_test
    boundary = '4'
  []
  [temp_avg5]
    type = SideAverageValue
    variable = temp_test
    boundary = '5'
  []
  [temp_avg6]
    type = SideAverageValue
    variable = temp_test
    boundary = '6'
  []
  [temp_avg7]
    type = SideAverageValue
    variable = temp_test
    boundary = '7'
  []
  [temp_avg8]
    type = SideAverageValue
    variable = temp_test
    boundary = '8'
  []
  [pressure_avg1]
    type = SideAverageValue
    variable = pressure_test
    boundary = '1'
  []
  [pressure_avg2]
    type = SideAverageValue
    variable = pressure_test
    boundary = '2'
  []
  [pressure_avg3]
    type = SideAverageValue
    variable = pressure_test
    boundary = '3'
  []
  [pressure_avg4]
    type = SideAverageValue
    variable = pressure_test
    boundary = '4'
  []
  [pressure_avg5]
    type = SideAverageValue
    variable = pressure_test
    boundary = '5'
  []
  [pressure_avg6]
    type = SideAverageValue
    variable = pressure_test
    boundary = '6'
  []
  [pressure_avg7]
    type = SideAverageValue
    variable = pressure_test
    boundary = '7'
  []
  [pressure_avg8]
    type = SideAverageValue
    variable = pressure_test
    boundary = '8'
  []
[]
