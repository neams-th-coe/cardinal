[Mesh]
  type = FileMesh
  file = ../meshes/pyramid.exo
[]

[AuxVariables]
  [usrwrk00]
  []
  [usrwrk01]
  []
  [usrwrk02]
  []
  [temp]
  []
  [pressure]
  []
  [velocity]
  []
  [x_velocity]
  []
  [y_velocity]
  []
  [z_velocity]
  []
  [x_velocity2]
  []
  [y_velocity2]
  []
  [z_velocity2]
  []
  [velocity_component]
  []
  [scalar01]
  []
  [scalar02]
  []
  [scalar03]
  []
[]

[ICs]
  [usrwrk00]
    type = FunctionIC
    variable = usrwrk00
    function = usrwrk00
  []
  [usrwrk01]
    type = FunctionIC
    variable = usrwrk01
    function = usrwrk01
  []
  [usrwrk02]
    type = FunctionIC
    variable = usrwrk02
    function = usrwrk02
  []
  [temp]
    type = FunctionIC
    variable = temp
    function = temp
  []
  [scalar01]
    type = FunctionIC
    variable = scalar01
    function = scalar01
  []
  [scalar02]
    type = FunctionIC
    variable = scalar02
    function = scalar02
  []
  [scalar03]
    type = FunctionIC
    variable = scalar03
    function = scalar03
  []
  [pressure]
    type = FunctionIC
    variable = pressure
    function = pressure
  []
  [velocity]
    type = FunctionIC
    variable = velocity
    function = velocity
  []
  [x_velocity]
    type = FunctionIC
    variable = x_velocity
    function = x_velocity
  []
  [y_velocity]
    type = FunctionIC
    variable = y_velocity
    function = y_velocity
  []
  [z_velocity]
    type = FunctionIC
    variable = z_velocity
    function = z_velocity
  []
  [x_velocity2]
    type = FunctionIC
    variable = x_velocity2
    function = x_velocity2
  []
  [y_velocity2]
    type = FunctionIC
    variable = y_velocity2
    function = y_velocity2
  []
  [z_velocity2]
    type = FunctionIC
    variable = z_velocity2
    function = z_velocity2
  []
  [velocity_component]
    type = FunctionIC
    variable = velocity_component
    function = velocity_component
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Functions]
  [usrwrk00]
    type = ParsedFunction
    expression = 'exp(x)+1'
  []
  [usrwrk01]
    type = ParsedFunction
    expression = 'exp(y)+1'
  []
  [usrwrk02]
    type = ParsedFunction
    expression = 'exp(z)+1'
  []
  [temp]
    type = ParsedFunction
    expression = 'exp(x)+sin(y)+x*y*z'
  []
  [scalar01]
    type = ParsedFunction
    expression = 'exp(x)+sin(y)+x*y*z+1'
  []
  [scalar02]
    type = ParsedFunction
    expression = 'exp(x)+sin(y)+x*y*z+2'
  []
  [scalar03]
    type = ParsedFunction
    expression = 'exp(x)+sin(y)+x*y*z+3'
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
  [x_velocity2]
    type = ParsedFunction
    expression = '(sin(x))^2'
  []
  [y_velocity2]
    type = ParsedFunction
    expression = '(y+1)^2'
  []
  [z_velocity2]
    type = ParsedFunction
    expression = '(exp(x*y*z))^2'
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

  [Quadrature]
    type = GAUSS_LOBATTO
    order = FIRST
  []
[]

[Outputs]
  csv = true
  execute_on = 'final'
[]

[Postprocessors]
  [usrwrk00_int]
    type = ElementIntegralVariablePostprocessor
    variable = usrwrk00
  []
  [usrwrk01_int]
    type = ElementIntegralVariablePostprocessor
    variable = usrwrk01
  []
  [usrwrk02_int]
    type = ElementIntegralVariablePostprocessor
    variable = usrwrk02
  []
  [temp_int]
    type = ElementIntegralVariablePostprocessor
    variable = temp
  []
  [s01_int]
    type = ElementIntegralVariablePostprocessor
    variable = scalar01
  []
  [s02_int]
    type = ElementIntegralVariablePostprocessor
    variable = scalar02
  []
  [s03_int]
    type = ElementIntegralVariablePostprocessor
    variable = scalar03
  []
  [pressure_int]
    type = ElementIntegralVariablePostprocessor
    variable = pressure
  []
  [velocity_int]
    type = ElementIntegralVariablePostprocessor
    variable = velocity
  []
  [x_velocity_int]
    type = ElementIntegralVariablePostprocessor
    variable = x_velocity
  []
  [y_velocity_int]
    type = ElementIntegralVariablePostprocessor
    variable = y_velocity
  []
  [z_velocity_int]
    type = ElementIntegralVariablePostprocessor
    variable = z_velocity
  []
  [x_velocity_int2]
    type = ElementIntegralVariablePostprocessor
    variable = x_velocity2
  []
  [y_velocity_int2]
    type = ElementIntegralVariablePostprocessor
    variable = y_velocity2
  []
  [z_velocity_int2]
    type = ElementIntegralVariablePostprocessor
    variable = z_velocity2
  []
  [velocity_component]
    type = ElementIntegralVariablePostprocessor
    variable = velocity_component
  []
  [volume]
    type = VolumePostprocessor
  []
[]
