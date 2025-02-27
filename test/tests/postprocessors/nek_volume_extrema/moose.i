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
  [z_velocity_test_shift]
  []
[]

[AuxKernels]
  [temp_test]
    type = FunctionAux
    variable = temp_test
    function = temp
  []
  [pressure_test]
    type = FunctionAux
    variable = pressure_test
    function = pressure
  []
  [velocity_test]
    type = FunctionAux
    variable = velocity_test
    function = velocity
  []
  [x_velocity_test]
    type = FunctionAux
    variable = x_velocity_test
    function = x_velocity
  []
  [y_velocity_test]
    type = FunctionAux
    variable = y_velocity_test
    function = y_velocity
  []
  [z_velocity_test]
    type = FunctionAux
    variable = z_velocity_test
    function = z_velocity
  []
  [z_velocity_test_shift]
    type = FunctionAux
    variable = z_velocity_test_shift
    function = z_velocity_shift
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Functions]
  [temp]
    type = ParsedFunction
    expression = 'exp(x)+sin(y)+x*y*z'
  []
  [pressure]
    type = ParsedFunction
    expression = 'x+y+z*z+exp(x)'
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
  [z_velocity_shift]
    type = ParsedFunction
    expression = 'exp(x*y*z) - (x*x+10*y+7*t)'
  []
[]

[Executioner]
  type = Transient
  dt = 5e-4
  num_steps = 1
[]

[Outputs]
  csv = true
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
  [max_velocity]
    type = NodalExtremeValue
    variable = velocity_test
    value_type = max
  []
  [min_velocity]
    type = NodalExtremeValue
    variable = velocity_test
    value_type = min
  []
  [max_x_velocity]
    type = NodalExtremeValue
    variable = x_velocity_test
    value_type = max
  []
  [min_x_velocity]
    type = NodalExtremeValue
    variable = x_velocity_test
    value_type = min
  []
  [max_y_velocity]
    type = NodalExtremeValue
    variable = y_velocity_test
    value_type = max
  []
  [min_y_velocity]
    type = NodalExtremeValue
    variable = y_velocity_test
    value_type = min
  []
  [max_z_velocity]
    type = NodalExtremeValue
    variable = z_velocity_test
    value_type = max
  []
  [min_z_velocity]
    type = NodalExtremeValue
    variable = z_velocity_test
    value_type = min
  []
  [max_z_velocity_shift]
    type = NodalExtremeValue
    variable = z_velocity_test_shift
    value_type = max
  []
  [min_z_velocity_shift]
    type = NodalExtremeValue
    variable = z_velocity_test_shift
    value_type = min
  []
[]
