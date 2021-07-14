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
  [x_velocity_test]
  []
  [y_velocity_test]
  []
  [z_velocity_test]
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
  [x_velocity_test]
    type = FunctionIC
    variable = x_velocity_test
    function = x_velocity
  []
  [y_velocity_test]
    type = FunctionIC
    variable = y_velocity_test
    function = y_velocity
  []
  [z_velocity_test]
    type = FunctionIC
    variable = z_velocity_test
    function = z_velocity
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
  [x_velocity]
    type = ParsedFunction
    value = 'sin(x)'
  []
  [y_velocity]
    type = ParsedFunction
    value = 'y+1'
  []
  [z_velocity]
    type = ParsedFunction
    value = 'exp(x*y*z)'
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
  [x_velocity_avg1]
    type = SideAverageValue
    variable = x_velocity_test
    boundary = '1'
  []
  [x_velocity_avg2]
    type = SideAverageValue
    variable = x_velocity_test
    boundary = '2'
  []
  [x_velocity_avg3]
    type = SideAverageValue
    variable = x_velocity_test
    boundary = '3'
  []
  [x_velocity_avg4]
    type = SideAverageValue
    variable = x_velocity_test
    boundary = '4'
  []
  [x_velocity_avg5]
    type = SideAverageValue
    variable = x_velocity_test
    boundary = '5'
  []
  [x_velocity_avg6]
    type = SideAverageValue
    variable = x_velocity_test
    boundary = '6'
  []
  [x_velocity_avg7]
    type = SideAverageValue
    variable = x_velocity_test
    boundary = '7'
  []
  [x_velocity_avg8]
    type = SideAverageValue
    variable = x_velocity_test
    boundary = '8'
  []
  [y_velocity_avg1]
    type = SideAverageValue
    variable = y_velocity_test
    boundary = '1'
  []
  [y_velocity_avg2]
    type = SideAverageValue
    variable = y_velocity_test
    boundary = '2'
  []
  [y_velocity_avg3]
    type = SideAverageValue
    variable = y_velocity_test
    boundary = '3'
  []
  [y_velocity_avg4]
    type = SideAverageValue
    variable = y_velocity_test
    boundary = '4'
  []
  [y_velocity_avg5]
    type = SideAverageValue
    variable = y_velocity_test
    boundary = '5'
  []
  [y_velocity_avg6]
    type = SideAverageValue
    variable = y_velocity_test
    boundary = '6'
  []
  [y_velocity_avg7]
    type = SideAverageValue
    variable = y_velocity_test
    boundary = '7'
  []
  [y_velocity_avg8]
    type = SideAverageValue
    variable = y_velocity_test
    boundary = '8'
  []
  [z_velocity_avg1]
    type = SideAverageValue
    variable = z_velocity_test
    boundary = '1'
  []
  [z_velocity_avg2]
    type = SideAverageValue
    variable = z_velocity_test
    boundary = '2'
  []
  [z_velocity_avg3]
    type = SideAverageValue
    variable = z_velocity_test
    boundary = '3'
  []
  [z_velocity_avg4]
    type = SideAverageValue
    variable = z_velocity_test
    boundary = '4'
  []
  [z_velocity_avg5]
    type = SideAverageValue
    variable = z_velocity_test
    boundary = '5'
  []
  [z_velocity_avg6]
    type = SideAverageValue
    variable = z_velocity_test
    boundary = '6'
  []
  [z_velocity_avg7]
    type = SideAverageValue
    variable = z_velocity_test
    boundary = '7'
  []
  [z_velocity_avg8]
    type = SideAverageValue
    variable = z_velocity_test
    boundary = '8'
  []
[]
