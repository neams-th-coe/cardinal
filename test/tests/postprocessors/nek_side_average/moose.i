[Mesh]
  type = FileMesh
  file = ../meshes/pyramid.exo
[]

[AuxVariables]
  [temp_test]
  []
[]

[ICs]
  [temp_test]
    type = FunctionIC
    variable = temp_test
    function = temp
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
[]
