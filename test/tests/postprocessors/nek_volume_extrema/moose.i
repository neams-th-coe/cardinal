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
    value = 'x+y+z*z+exp(x)'
  []
[]

[Executioner]
  type = Transient
  dt = 5e-4
  num_steps = 1
[]

[Outputs]
  [out]
    type = CSV
    execute_on = 'final'
  []
[]

[Postprocessors]
  [max_temp]
    type = NodalExtremeValue
    variable = temp_test
    value_type = max
  []
  [min_temp]
    type = NodalExtremeValue
    variable = temp_test
    value_type = min
  []
  [max_p]
    type = NodalExtremeValue
    variable = pressure_test
    value_type = max
  []
  [min_p]
    type = NodalExtremeValue
    variable = pressure_test
    value_type = min
  []
[]
