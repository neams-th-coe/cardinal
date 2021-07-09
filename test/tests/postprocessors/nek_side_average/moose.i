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
    value = 'exp(x)+exp(y)+exp(z)'
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
  [velocity_avg1]
    type = SideAverageValue
    variable = velocity_test
    boundary = '1'
  []
  [velocity_avg2]
    type = SideAverageValue
    variable = velocity_test
    boundary = '2'
  []
  [velocity_avg3]
    type = SideAverageValue
    variable = velocity_test
    boundary = '3'
  []
  [velocity_avg4]
    type = SideAverageValue
    variable = velocity_test
    boundary = '4'
  []
  [velocity_avg5]
    type = SideAverageValue
    variable = velocity_test
    boundary = '5'
  []
  [velocity_avg6]
    type = SideAverageValue
    variable = velocity_test
    boundary = '6'
  []
  [velocity_avg7]
    type = SideAverageValue
    variable = velocity_test
    boundary = '7'
  []
  [velocity_avg8]
    type = SideAverageValue
    variable = velocity_test
    boundary = '8'
  []
[]
