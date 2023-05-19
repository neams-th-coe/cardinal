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
    function = f
  []
  [pressure_test]
    type = FunctionIC
    variable = pressure_test
    function = f
  []
  [velocity_test]
    type = FunctionIC
    variable = velocity_test
    function = velocity
  []
  [x_velocity_test]
    type = FunctionIC
    variable = x_velocity_test
    function = velocity_component
  []
  [y_velocity_test]
    type = FunctionIC
    variable = y_velocity_test
    function = velocity_component
  []
  [z_velocity_test]
    type = FunctionIC
    variable = z_velocity_test
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
  [f]
    type = ParsedFunction
    expression = 'exp(x)+sin(y)+x*y*z'
  []
  [velocity]
     type = ParsedFunction
     expression = 'sqrt(3.0)*exp(-r*r)'
     symbol_names = 'r'
     symbol_values = 'r'
  []
  [r]
    type = ParsedFunction
    expression = 'sqrt(x*x+y*y+z*z)'
  []
  [velocity_component]
    type = ParsedFunction
    expression = 'exp(-r*r)'
    symbol_names = 'r'
    symbol_values = 'r'
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
  [max_temp_side1]
    type = NodalExtremeValue
    variable = temp_test
    boundary = '1'
  []
  [max_temp_side2]
    type = NodalExtremeValue
    variable = temp_test
    boundary = '2'
  []
  [max_temp_side3]
    type = NodalExtremeValue
    variable = temp_test
    boundary = '3'
  []
  [max_temp_side4]
    type = NodalExtremeValue
    variable = temp_test
    boundary = '4'
  []
  [max_temp_side5]
    type = NodalExtremeValue
    variable = temp_test
    boundary = '5'
  []
  [max_temp_side6]
    type = NodalExtremeValue
    variable = temp_test
    boundary = '6'
  []
  [max_temp_side7]
    type = NodalExtremeValue
    variable = temp_test
    boundary = '7'
  []
  [max_temp_side8]
    type = NodalExtremeValue
    variable = temp_test
    boundary = '8'
  []
  [min_temp_side1]
    type = NodalExtremeValue
    variable = temp_test
    boundary = '1'
    value_type = min
  []
  [min_temp_side2]
    type = NodalExtremeValue
    variable = temp_test
    boundary = '2'
    value_type = min
  []
  [min_temp_side3]
    type = NodalExtremeValue
    variable = temp_test
    boundary = '3'
    value_type = min
  []
  [min_temp_side4]
    type = NodalExtremeValue
    variable = temp_test
    boundary = '4'
    value_type = min
  []
  [min_temp_side5]
    type = NodalExtremeValue
    variable = temp_test
    boundary = '5'
    value_type = min
  []
  [min_temp_side6]
    type = NodalExtremeValue
    variable = temp_test
    boundary = '6'
    value_type = min
  []
  [min_temp_side7]
    type = NodalExtremeValue
    variable = temp_test
    boundary = '7'
    value_type = min
  []
  [min_temp_side8]
    type = NodalExtremeValue
    variable = temp_test
    boundary = '8'
    value_type = min
  []

  [max_pressure_side1]
    type = NodalExtremeValue
    variable = pressure_test
    boundary = '1'
  []
  [max_pressure_side2]
    type = NodalExtremeValue
    variable = pressure_test
    boundary = '2'
  []
  [max_pressure_side3]
    type = NodalExtremeValue
    variable = pressure_test
    boundary = '3'
  []
  [max_pressure_side4]
    type = NodalExtremeValue
    variable = pressure_test
    boundary = '4'
  []
  [max_pressure_side5]
    type = NodalExtremeValue
    variable = pressure_test
    boundary = '5'
  []
  [max_pressure_side6]
    type = NodalExtremeValue
    variable = pressure_test
    boundary = '6'
  []
  [max_pressure_side7]
    type = NodalExtremeValue
    variable = pressure_test
    boundary = '7'
  []
  [max_pressure_side8]
    type = NodalExtremeValue
    variable = pressure_test
    boundary = '8'
  []
  [min_pressure_side1]
    type = NodalExtremeValue
    variable = pressure_test
    boundary = '1'
    value_type = min
  []
  [min_pressure_side2]
    type = NodalExtremeValue
    variable = pressure_test
    boundary = '2'
    value_type = min
  []
  [min_pressure_side3]
    type = NodalExtremeValue
    variable = pressure_test
    boundary = '3'
    value_type = min
  []
  [min_pressure_side4]
    type = NodalExtremeValue
    variable = pressure_test
    boundary = '4'
    value_type = min
  []
  [min_pressure_side5]
    type = NodalExtremeValue
    variable = pressure_test
    boundary = '5'
    value_type = min
  []
  [min_pressure_side6]
    type = NodalExtremeValue
    variable = pressure_test
    boundary = '6'
    value_type = min
  []
  [min_pressure_side7]
    type = NodalExtremeValue
    variable = pressure_test
    boundary = '7'
    value_type = min
  []
  [min_pressure_side8]
    type = NodalExtremeValue
    variable = pressure_test
    boundary = '8'
    value_type = min
  []

  [max_velocity_side1]
    type = NodalExtremeValue
    variable = velocity_test
    boundary = '1'
  []
  [max_velocity_side2]
    type = NodalExtremeValue
    variable = velocity_test
    boundary = '2'
  []
  [max_velocity_side3]
    type = NodalExtremeValue
    variable = velocity_test
    boundary = '3'
  []
  [max_velocity_side4]
    type = NodalExtremeValue
    variable = velocity_test
    boundary = '4'
  []
  [max_velocity_side5]
    type = NodalExtremeValue
    variable = velocity_test
    boundary = '5'
  []
  [max_velocity_side6]
    type = NodalExtremeValue
    variable = velocity_test
    boundary = '6'
  []
  [max_velocity_side7]
    type = NodalExtremeValue
    variable = velocity_test
    boundary = '7'
  []
  [max_velocity_side8]
    type = NodalExtremeValue
    variable = velocity_test
    boundary = '8'
  []
  [min_velocity_side1]
    type = NodalExtremeValue
    variable = velocity_test
    boundary = '1'
    value_type = min
  []
  [min_velocity_side2]
    type = NodalExtremeValue
    variable = velocity_test
    boundary = '2'
    value_type = min
  []
  [min_velocity_side3]
    type = NodalExtremeValue
    variable = velocity_test
    boundary = '3'
    value_type = min
  []
  [min_velocity_side4]
    type = NodalExtremeValue
    variable = velocity_test
    boundary = '4'
    value_type = min
  []
  [min_velocity_side5]
    type = NodalExtremeValue
    variable = velocity_test
    boundary = '5'
    value_type = min
  []
  [min_velocity_side6]
    type = NodalExtremeValue
    variable = velocity_test
    boundary = '6'
    value_type = min
  []
  [min_velocity_side7]
    type = NodalExtremeValue
    variable = velocity_test
    boundary = '7'
    value_type = min
  []
  [min_velocity_side8]
    type = NodalExtremeValue
    variable = velocity_test
    boundary = '8'
    value_type = min
  []
  [min_x_velocity_side1]
    type = NodalExtremeValue
    variable = x_velocity_test
    boundary = '1'
    value_type = min
  []
  [min_x_velocity_side2]
    type = NodalExtremeValue
    variable = x_velocity_test
    boundary = '2'
    value_type = min
  []
  [min_x_velocity_side3]
    type = NodalExtremeValue
    variable = x_velocity_test
    boundary = '3'
    value_type = min
  []
  [min_x_velocity_side4]
    type = NodalExtremeValue
    variable = x_velocity_test
    boundary = '4'
    value_type = min
  []
  [min_x_velocity_side5]
    type = NodalExtremeValue
    variable = x_velocity_test
    boundary = '5'
    value_type = min
  []
  [min_x_velocity_side6]
    type = NodalExtremeValue
    variable = x_velocity_test
    boundary = '6'
    value_type = min
  []
  [min_x_velocity_side7]
    type = NodalExtremeValue
    variable = x_velocity_test
    boundary = '7'
    value_type = min
  []
  [min_x_velocity_side8]
    type = NodalExtremeValue
    variable = x_velocity_test
    boundary = '8'
    value_type = min
  []
  [min_y_velocity_side1]
    type = NodalExtremeValue
    variable = y_velocity_test
    boundary = '1'
    value_type = min
  []
  [min_y_velocity_side2]
    type = NodalExtremeValue
    variable = y_velocity_test
    boundary = '2'
    value_type = min
  []
  [min_y_velocity_side3]
    type = NodalExtremeValue
    variable = y_velocity_test
    boundary = '3'
    value_type = min
  []
  [min_y_velocity_side4]
    type = NodalExtremeValue
    variable = y_velocity_test
    boundary = '4'
    value_type = min
  []
  [min_y_velocity_side5]
    type = NodalExtremeValue
    variable = y_velocity_test
    boundary = '5'
    value_type = min
  []
  [min_y_velocity_side6]
    type = NodalExtremeValue
    variable = y_velocity_test
    boundary = '6'
    value_type = min
  []
  [min_y_velocity_side7]
    type = NodalExtremeValue
    variable = y_velocity_test
    boundary = '7'
    value_type = min
  []
  [min_y_velocity_side8]
    type = NodalExtremeValue
    variable = y_velocity_test
    boundary = '8'
    value_type = min
  []
  [min_z_velocity_side1]
    type = NodalExtremeValue
    variable = z_velocity_test
    boundary = '1'
    value_type = min
  []
  [min_z_velocity_side2]
    type = NodalExtremeValue
    variable = z_velocity_test
    boundary = '2'
    value_type = min
  []
  [min_z_velocity_side3]
    type = NodalExtremeValue
    variable = z_velocity_test
    boundary = '3'
    value_type = min
  []
  [min_z_velocity_side4]
    type = NodalExtremeValue
    variable = z_velocity_test
    boundary = '4'
    value_type = min
  []
  [min_z_velocity_side5]
    type = NodalExtremeValue
    variable = z_velocity_test
    boundary = '5'
    value_type = min
  []
  [min_z_velocity_side6]
    type = NodalExtremeValue
    variable = z_velocity_test
    boundary = '6'
    value_type = min
  []
  [min_z_velocity_side7]
    type = NodalExtremeValue
    variable = z_velocity_test
    boundary = '7'
    value_type = min
  []
  [min_z_velocity_side8]
    type = NodalExtremeValue
    variable = z_velocity_test
    boundary = '8'
    value_type = min
  []
  [max_x_velocity_side1]
    type = NodalExtremeValue
    variable = x_velocity_test
    boundary = '1'
    value_type = max
  []
  [max_x_velocity_side2]
    type = NodalExtremeValue
    variable = x_velocity_test
    boundary = '2'
    value_type = max
  []
  [max_x_velocity_side3]
    type = NodalExtremeValue
    variable = x_velocity_test
    boundary = '3'
    value_type = max
  []
  [max_x_velocity_side4]
    type = NodalExtremeValue
    variable = x_velocity_test
    boundary = '4'
    value_type = max
  []
  [max_x_velocity_side5]
    type = NodalExtremeValue
    variable = x_velocity_test
    boundary = '5'
    value_type = max
  []
  [max_x_velocity_side6]
    type = NodalExtremeValue
    variable = x_velocity_test
    boundary = '6'
    value_type = max
  []
  [max_x_velocity_side7]
    type = NodalExtremeValue
    variable = x_velocity_test
    boundary = '7'
    value_type = max
  []
  [max_x_velocity_side8]
    type = NodalExtremeValue
    variable = x_velocity_test
    boundary = '8'
    value_type = max
  []
  [max_y_velocity_side1]
    type = NodalExtremeValue
    variable = y_velocity_test
    boundary = '1'
    value_type = max
  []
  [max_y_velocity_side2]
    type = NodalExtremeValue
    variable = y_velocity_test
    boundary = '2'
    value_type = max
  []
  [max_y_velocity_side3]
    type = NodalExtremeValue
    variable = y_velocity_test
    boundary = '3'
    value_type = max
  []
  [max_y_velocity_side4]
    type = NodalExtremeValue
    variable = y_velocity_test
    boundary = '4'
    value_type = max
  []
  [max_y_velocity_side5]
    type = NodalExtremeValue
    variable = y_velocity_test
    boundary = '5'
    value_type = max
  []
  [max_y_velocity_side6]
    type = NodalExtremeValue
    variable = y_velocity_test
    boundary = '6'
    value_type = max
  []
  [max_y_velocity_side7]
    type = NodalExtremeValue
    variable = y_velocity_test
    boundary = '7'
    value_type = max
  []
  [max_y_velocity_side8]
    type = NodalExtremeValue
    variable = y_velocity_test
    boundary = '8'
    value_type = max
  []
  [max_z_velocity_side1]
    type = NodalExtremeValue
    variable = z_velocity_test
    boundary = '1'
    value_type = max
  []
  [max_z_velocity_side2]
    type = NodalExtremeValue
    variable = z_velocity_test
    boundary = '2'
    value_type = max
  []
  [max_z_velocity_side3]
    type = NodalExtremeValue
    variable = z_velocity_test
    boundary = '3'
    value_type = max
  []
  [max_z_velocity_side4]
    type = NodalExtremeValue
    variable = z_velocity_test
    boundary = '4'
    value_type = max
  []
  [max_z_velocity_side5]
    type = NodalExtremeValue
    variable = z_velocity_test
    boundary = '5'
    value_type = max
  []
  [max_z_velocity_side6]
    type = NodalExtremeValue
    variable = z_velocity_test
    boundary = '6'
    value_type = max
  []
  [max_z_velocity_side7]
    type = NodalExtremeValue
    variable = z_velocity_test
    boundary = '7'
    value_type = max
  []
  [max_z_velocity_side8]
    type = NodalExtremeValue
    variable = z_velocity_test
    boundary = '8'
    value_type = max
  []
[]
