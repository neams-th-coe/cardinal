L_ref = 5.0
U = 0.2

[Mesh]
  [f]
    type = FileMeshGenerator
    file = ../meshes/pyramid.exo
  []
[]

[Problem]
  type = FEProblem
  solve = false
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
  [temp_minus_f]
  []
[]

[ICs]
  [temp_test]
    type = FunctionIC
    variable = temp_test
    function = temp
  []
  [temp_minus_f]
    type = FunctionIC
    variable = temp_minus_f
    function = temp_minus_f
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

[AuxKernels]
  [temp_minus_f]
    type = FunctionAux
    variable = temp_minus_f
    function = temp_minus_f
  []
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
  [f] # express the function, nondimensional form
    type = ParsedFunction
    expression = '((x+y+z)*${L_ref}-30)/200'
  []
  [temp_minus_f]
    type = ParsedFunction
    expression = 'temp - f'
    symbol_names = 'temp f'
    symbol_values = 'temp f'
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
  [max_tempf]
    type = NodalExtremeValue
    variable = temp_minus_f
    value_type = max
    execute_on = timestep_end
  []
  [min_tempf]
    type = NodalExtremeValue
    variable = temp_minus_f
    value_type = min
    execute_on = timestep_end
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
[]
