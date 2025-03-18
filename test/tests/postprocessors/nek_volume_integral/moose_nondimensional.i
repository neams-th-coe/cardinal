[Mesh]
  [f]
    type = FileMeshGenerator
    file = ../meshes/pyramid.exo
  []
  [scale]
    type = TransformGenerator
    input = f
    transform = scale
    vector_value = '2 2 2'
  []
[]

[AuxVariables]
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
  [usrwrk00]
  []
  [usrwrk01]
  []
  [usrwrk02]
  []
[]

[ICs]
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
  [xx]
    type = ParsedFunction
    expression = 'x/2'
  []
  [yy]
    type = ParsedFunction
    expression = 'y/2'
  []
  [zz]
    type = ParsedFunction
    expression = 'z/2'
  []
  [temp]
    type = ParsedFunction
    expression = '(exp(xx)+sin(yy)+xx*yy*zz)*100+10'
    symbol_names = 'xx yy zz'
    symbol_values = 'xx yy zz'
  []
  [scalar01]
    type = ParsedFunction
    expression = '(exp(xx)+sin(yy)+xx*yy*zz+1)*150+15'
    symbol_names = 'xx yy zz'
    symbol_values = 'xx yy zz'
  []
  [scalar02]
    type = ParsedFunction
    expression = '(exp(xx)+sin(yy)+xx*yy*zz+2)*200+20'
    symbol_names = 'xx yy zz'
    symbol_values = 'xx yy zz'
  []
  [scalar03]
    type = ParsedFunction
    expression = '(exp(xx)+sin(yy)+xx*yy*zz+3)*300+30'
    symbol_names = 'xx yy zz'
    symbol_values = 'xx yy zz'
  []
  [pressure]
    type = ParsedFunction
    expression = '(exp(xx) + exp(yy) + exp(zz))*(1000*0.2*0.2)'
    symbol_names = 'xx yy zz'
    symbol_values = 'xx yy zz'
  []
  [velocity]
     type = ParsedFunction
     expression = 'sqrt(vx^2+vy^2+vz^2)'
     symbol_names = 'vx vy vz'
     symbol_values = 'x_velocity y_velocity z_velocity'
  []
  [x_velocity]
    type = ParsedFunction
    expression = 'sin(xx)*.2'
    symbol_names = 'xx yy zz'
    symbol_values = 'xx yy zz'
  []
  [y_velocity]
    type = ParsedFunction
    expression = '(yy+1)*.2'
    symbol_names = 'xx yy zz'
    symbol_values = 'xx yy zz'
  []
  [z_velocity]
    type = ParsedFunction
    expression = 'exp(xx*yy*zz)*.2'
    symbol_names = 'xx yy zz'
    symbol_values = 'xx yy zz'
  []
  [x_velocity2]
    type = ParsedFunction
    expression = '(sin(xx)*.2)^2'
    symbol_names = 'xx yy zz'
    symbol_values = 'xx yy zz'
  []
  [y_velocity2]
    type = ParsedFunction
    expression = '((yy+1)*.2)^2'
    symbol_names = 'xx yy zz'
    symbol_values = 'xx yy zz'
  []
  [z_velocity2]
    type = ParsedFunction
    expression = '(exp(xx*yy*zz)*.2)^2'
    symbol_names = 'xx yy zz'
    symbol_values = 'xx yy zz'
  []
  [velocity_component] # velocity along some generic direction (0.1, 0.2, -0.3)
    type = ParsedFunction
    expression = '(vel_x * 0.1 + vel_y * 0.2 + vel_z * -0.3) / sqrt(0.1*0.1 + 0.2*0.2 + 0.3*0.3)'
    symbol_names = 'vel_x vel_y vel_z'
    symbol_values = 'x_velocity y_velocity z_velocity'
  []
  [usrwrk00]
    type = ParsedFunction
    expression = 'exp(xx)+1'
    symbol_names = 'xx'
    symbol_values = 'xx'
  []
  [usrwrk01]
    type = ParsedFunction
    expression = 'exp(yy)+1'
    symbol_names = 'yy'
    symbol_values = 'yy'
  []
  [usrwrk02]
    type = ParsedFunction
    expression = 'exp(zz)+1'
    symbol_names = 'zz'
    symbol_values = 'zz'
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
  [volume]
    type = VolumePostprocessor
  []
  [temp_int]
    type = ElementIntegralVariablePostprocessor
    variable = temp
  []
  [u00_int]
    type = ElementIntegralVariablePostprocessor
    variable = usrwrk00
  []
  [u01_int]
    type = ElementIntegralVariablePostprocessor
    variable = usrwrk01
  []
  [u02_int]
    type = ElementIntegralVariablePostprocessor
    variable = usrwrk02
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
[]
