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
  [velocity_component]
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
  [velocity_component]
    type = FunctionIC
    variable = velocity_component
    function = velocity_component
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
  [pressure]
    type = ParsedFunction
    expression = 'exp(x) + exp(y) + exp(z)'
  []
  [velocity]
     type = ParsedFunction
     expression = 'sqrt(sin(x)*sin(x)+(y+1)*(y+1)+exp(x*y*z)*exp(x*y*z))'
  []
  [x_velocity]
    type = ParsedFunction
    expression = 'sin(x)'
  []
  [y_velocity]
    type = ParsedFunction
    expression = 'y+1'
  []
  [z_velocity]
    type = ParsedFunction
    expression = 'exp(x*y*z)'
  []
  [velocity_component] # velocity along some generic direction (0.1, 0.2, -0.3)
    type = ParsedFunction
    expression = '(vel_x * 0.1 + vel_y * 0.2 + vel_z * -0.3) / sqrt(0.1*0.1 + 0.2*0.2 + 0.3*0.3)'
    symbol_names = 'vel_x vel_y vel_z'
    symbol_values = 'x_velocity y_velocity z_velocity'
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
  [temp_average]
    type = ElementAverageValue
    variable = temp_test
  []
  [pressure_average]
    type = ElementAverageValue
    variable = pressure_test
  []
  [velocity_average]
    type = ElementAverageValue
    variable = velocity_test
  []
  [x_velocity_average]
    type = ElementAverageValue
    variable = x_velocity_test
  []
  [y_velocity_average]
    type = ElementAverageValue
    variable = y_velocity_test
  []
  [z_velocity_average]
    type = ElementAverageValue
    variable = z_velocity_test
  []
  [velocity_component]
    type = ElementAverageValue
    variable = velocity_component
  []
[]
