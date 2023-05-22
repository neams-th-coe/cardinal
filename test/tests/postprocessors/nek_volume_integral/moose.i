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
    expression = 'exp(x)+exp(y)+exp(z)'
  []
  [velocity]
    type = ParsedFunction
    expression = 'sqrt(sin(x)*sin(x)+(y+1)*(y+1)+exp(x*y*z)*exp(x*y*z))'
  []
  [velocity_x]
    type = ParsedFunction
    expression = 'sin(x)'
  []
  [velocity_y]
    type = ParsedFunction
    expression = 'y+1'
  []
  [velocity_z]
    type = ParsedFunction
    expression = 'exp(x*y*z)'
  []
  [velocity_component] # velocity along some generic direction (0.1, 0.2, 0.3)
    type = ParsedFunction
    expression = '(vel_x * 0.1 + vel_y * 0.2 + vel_z * 0.3) / sqrt(0.1*0.1 + 0.2*0.2 + 0.3*0.3)'
    symbol_names = 'vel_x vel_y vel_z'
    symbol_values = 'velocity_x velocity_y velocity_z'
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
  [volume]
    type = VolumePostprocessor
  []
  [temp_integral]
    type = ElementIntegralVariablePostprocessor
    variable = temp_test
  []
  [pressure_integral]
    type = ElementIntegralVariablePostprocessor
    variable = pressure_test
  []
  [velocity_integral]
    type = ElementIntegralVariablePostprocessor
    variable = velocity_test
  []
  [velocity_component]
    type = ElementIntegralVariablePostprocessor
    variable = velocity_component
  []
[]
