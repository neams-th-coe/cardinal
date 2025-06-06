[Problem]
  type = NekRSProblem
  casename = 'pyramid'
[]

[Mesh]
  type = NekRSMesh
  boundary = '1 2 3 4 5 6 7 8'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  csv = true
  execute_on = 'final'
[]

[Postprocessors]
  [max_temp_side1]
    type = NekSideExtremeValue
    field = temperature
    boundary = '1'
    value_type = max
  []
  [max_temp_side2]
    type = NekSideExtremeValue
    field = temperature
    boundary = '2'
    value_type = max
  []
  [max_temp_side3]
    type = NekSideExtremeValue
    field = temperature
    boundary = '3'
    value_type = max
  []
  [max_temp_side4]
    type = NekSideExtremeValue
    field = temperature
    boundary = '4'
    value_type = max
  []
  [max_temp_side5]
    type = NekSideExtremeValue
    field = temperature
    boundary = '5'
    value_type = max
  []
  [max_temp_side6]
    type = NekSideExtremeValue
    field = temperature
    boundary = '6'
    value_type = max
  []
  [max_temp_side7]
    type = NekSideExtremeValue
    field = temperature
    boundary = '7'
    value_type = max
  []
  [max_temp_side8]
    type = NekSideExtremeValue
    field = temperature
    boundary = '8'
    value_type = max
  []
  [min_temp_side1]
    type = NekSideExtremeValue
    field = temperature
    boundary = '1'
    value_type = min
  []
  [min_temp_side2]
    type = NekSideExtremeValue
    field = temperature
    boundary = '2'
    value_type = min
  []
  [min_temp_side3]
    type = NekSideExtremeValue
    field = temperature
    boundary = '3'
    value_type = min
  []
  [min_temp_side4]
    type = NekSideExtremeValue
    field = temperature
    boundary = '4'
    value_type = min
  []
  [min_temp_side5]
    type = NekSideExtremeValue
    field = temperature
    boundary = '5'
    value_type = min
  []
  [min_temp_side6]
    type = NekSideExtremeValue
    field = temperature
    boundary = '6'
    value_type = min
  []
  [min_temp_side7]
    type = NekSideExtremeValue
    field = temperature
    boundary = '7'
    value_type = min
  []
  [min_temp_side8]
    type = NekSideExtremeValue
    field = temperature
    boundary = '8'
    value_type = min
  []

  [max_pressure_side1]
    type = NekSideExtremeValue
    field = pressure
    boundary = '1'
  []
  [max_pressure_side2]
    type = NekSideExtremeValue
    field = pressure
    boundary = '2'
  []
  [max_pressure_side3]
    type = NekSideExtremeValue
    field = pressure
    boundary = '3'
  []
  [max_pressure_side4]
    type = NekSideExtremeValue
    field = pressure
    boundary = '4'
  []
  [max_pressure_side5]
    type = NekSideExtremeValue
    field = pressure
    boundary = '5'
  []
  [max_pressure_side6]
    type = NekSideExtremeValue
    field = pressure
    boundary = '6'
  []
  [max_pressure_side7]
    type = NekSideExtremeValue
    field = pressure
    boundary = '7'
  []
  [max_pressure_side8]
    type = NekSideExtremeValue
    field = pressure
    boundary = '8'
  []
  [min_pressure_side1]
    type = NekSideExtremeValue
    field = pressure
    boundary = '1'
    value_type = min
  []
  [min_pressure_side2]
    type = NekSideExtremeValue
    field = pressure
    boundary = '2'
    value_type = min
  []
  [min_pressure_side3]
    type = NekSideExtremeValue
    field = pressure
    boundary = '3'
    value_type = min
  []
  [min_pressure_side4]
    type = NekSideExtremeValue
    field = pressure
    boundary = '4'
    value_type = min
  []
  [min_pressure_side5]
    type = NekSideExtremeValue
    field = pressure
    boundary = '5'
    value_type = min
  []
  [min_pressure_side6]
    type = NekSideExtremeValue
    field = pressure
    boundary = '6'
    value_type = min
  []
  [min_pressure_side7]
    type = NekSideExtremeValue
    field = pressure
    boundary = '7'
    value_type = min
  []
  [min_pressure_side8]
    type = NekSideExtremeValue
    field = pressure
    boundary = '8'
    value_type = min
  []

  [max_velocity_side1]
    type = NekSideExtremeValue
    field = velocity
    boundary = '1'
    value_type = max
  []
  [max_velocity_side2]
    type = NekSideExtremeValue
    field = velocity
    boundary = '2'
    value_type = max
  []
  [max_velocity_side3]
    type = NekSideExtremeValue
    field = velocity
    boundary = '3'
    value_type = max
  []
  [max_velocity_side4]
    type = NekSideExtremeValue
    field = velocity
    boundary = '4'
    value_type = max
  []
  [max_velocity_side5]
    type = NekSideExtremeValue
    field = velocity
    boundary = '5'
    value_type = max
  []
  [max_velocity_side6]
    type = NekSideExtremeValue
    field = velocity
    boundary = '6'
    value_type = max
  []
  [max_velocity_side7]
    type = NekSideExtremeValue
    field = velocity
    boundary = '7'
    value_type = max
  []
  [max_velocity_side8]
    type = NekSideExtremeValue
    field = velocity
    boundary = '8'
    value_type = max
  []
  [min_velocity_side1]
    type = NekSideExtremeValue
    field = velocity
    boundary = '1'
    value_type = min
  []
  [min_velocity_side2]
    type = NekSideExtremeValue
    field = velocity
    boundary = '2'
    value_type = min
  []
  [min_velocity_side3]
    type = NekSideExtremeValue
    field = velocity
    boundary = '3'
    value_type = min
  []
  [min_velocity_side4]
    type = NekSideExtremeValue
    field = velocity
    boundary = '4'
    value_type = min
  []
  [min_velocity_side5]
    type = NekSideExtremeValue
    field = velocity
    boundary = '5'
    value_type = min
  []
  [min_velocity_side6]
    type = NekSideExtremeValue
    field = velocity
    boundary = '6'
    value_type = min
  []
  [min_velocity_side7]
    type = NekSideExtremeValue
    field = velocity
    boundary = '7'
    value_type = min
  []
  [min_velocity_side8]
    type = NekSideExtremeValue
    field = velocity
    boundary = '8'
    value_type = min
  []

  [max_x_velocity_side1]
    type = NekSideExtremeValue
    field = velocity_x
    boundary = '1'
    value_type = max
  []
  [max_x_velocity_side2]
    type = NekSideExtremeValue
    field = velocity_x
    boundary = '2'
    value_type = max
  []
  [max_x_velocity_side3]
    type = NekSideExtremeValue
    field = velocity_x
    boundary = '3'
    value_type = max
  []
  [max_x_velocity_side4]
    type = NekSideExtremeValue
    field = velocity_x
    boundary = '4'
    value_type = max
  []
  [max_x_velocity_side5]
    type = NekSideExtremeValue
    field = velocity_x
    boundary = '5'
    value_type = max
  []
  [max_x_velocity_side6]
    type = NekSideExtremeValue
    field = velocity_x
    boundary = '6'
    value_type = max
  []
  [max_x_velocity_side7]
    type = NekSideExtremeValue
    field = velocity_x
    boundary = '7'
    value_type = max
  []
  [max_x_velocity_side8]
    type = NekSideExtremeValue
    field = velocity_x
    boundary = '8'
    value_type = max
  []
  [min_x_velocity_side1]
    type = NekSideExtremeValue
    field = velocity_x
    boundary = '1'
    value_type = min
  []
  [min_x_velocity_side2]
    type = NekSideExtremeValue
    field = velocity_x
    boundary = '2'
    value_type = min
  []
  [min_x_velocity_side3]
    type = NekSideExtremeValue
    field = velocity_x
    boundary = '3'
    value_type = min
  []
  [min_x_velocity_side4]
    type = NekSideExtremeValue
    field = velocity_x
    boundary = '4'
    value_type = min
  []
  [min_x_velocity_side5]
    type = NekSideExtremeValue
    field = velocity_x
    boundary = '5'
    value_type = min
  []
  [min_x_velocity_side6]
    type = NekSideExtremeValue
    field = velocity_x
    boundary = '6'
    value_type = min
  []
  [min_x_velocity_side7]
    type = NekSideExtremeValue
    field = velocity_x
    boundary = '7'
    value_type = min
  []
  [min_x_velocity_side8]
    type = NekSideExtremeValue
    field = velocity_x
    boundary = '8'
    value_type = min
  []
  [max_y_velocity_side1]
    type = NekSideExtremeValue
    field = velocity_y
    boundary = '1'
    value_type = max
  []
  [max_y_velocity_side2]
    type = NekSideExtremeValue
    field = velocity_y
    boundary = '2'
    value_type = max
  []
  [max_y_velocity_side3]
    type = NekSideExtremeValue
    field = velocity_y
    boundary = '3'
    value_type = max
  []
  [max_y_velocity_side4]
    type = NekSideExtremeValue
    field = velocity_y
    boundary = '4'
    value_type = max
  []
  [max_y_velocity_side5]
    type = NekSideExtremeValue
    field = velocity_y
    boundary = '5'
    value_type = max
  []
  [max_y_velocity_side6]
    type = NekSideExtremeValue
    field = velocity_y
    boundary = '6'
    value_type = max
  []
  [max_y_velocity_side7]
    type = NekSideExtremeValue
    field = velocity_y
    boundary = '7'
    value_type = max
  []
  [max_y_velocity_side8]
    type = NekSideExtremeValue
    field = velocity_y
    boundary = '8'
    value_type = max
  []
  [min_y_velocity_side1]
    type = NekSideExtremeValue
    field = velocity_y
    boundary = '1'
    value_type = min
  []
  [min_y_velocity_side2]
    type = NekSideExtremeValue
    field = velocity_y
    boundary = '2'
    value_type = min
  []
  [min_y_velocity_side3]
    type = NekSideExtremeValue
    field = velocity_y
    boundary = '3'
    value_type = min
  []
  [min_y_velocity_side4]
    type = NekSideExtremeValue
    field = velocity_y
    boundary = '4'
    value_type = min
  []
  [min_y_velocity_side5]
    type = NekSideExtremeValue
    field = velocity_y
    boundary = '5'
    value_type = min
  []
  [min_y_velocity_side6]
    type = NekSideExtremeValue
    field = velocity_y
    boundary = '6'
    value_type = min
  []
  [min_y_velocity_side7]
    type = NekSideExtremeValue
    field = velocity_y
    boundary = '7'
    value_type = min
  []
  [min_y_velocity_side8]
    type = NekSideExtremeValue
    field = velocity_y
    boundary = '8'
    value_type = min
  []
  [max_z_velocity_side1]
    type = NekSideExtremeValue
    field = velocity_z
    boundary = '1'
    value_type = max
  []
  [max_z_velocity_side2]
    type = NekSideExtremeValue
    field = velocity_z
    boundary = '2'
    value_type = max
  []
  [max_z_velocity_side3]
    type = NekSideExtremeValue
    field = velocity_z
    boundary = '3'
    value_type = max
  []
  [max_z_velocity_side4]
    type = NekSideExtremeValue
    field = velocity_z
    boundary = '4'
    value_type = max
  []
  [max_z_velocity_side5]
    type = NekSideExtremeValue
    field = velocity_z
    boundary = '5'
    value_type = max
  []
  [max_z_velocity_side6]
    type = NekSideExtremeValue
    field = velocity_z
    boundary = '6'
    value_type = max
  []
  [max_z_velocity_side7]
    type = NekSideExtremeValue
    field = velocity_z
    boundary = '7'
    value_type = max
  []
  [max_z_velocity_side8]
    type = NekSideExtremeValue
    field = velocity_z
    boundary = '8'
    value_type = max
  []
  [min_z_velocity_side1]
    type = NekSideExtremeValue
    field = velocity_z
    boundary = '1'
    value_type = min
  []
  [min_z_velocity_side2]
    type = NekSideExtremeValue
    field = velocity_z
    boundary = '2'
    value_type = min
  []
  [min_z_velocity_side3]
    type = NekSideExtremeValue
    field = velocity_z
    boundary = '3'
    value_type = min
  []
  [min_z_velocity_side4]
    type = NekSideExtremeValue
    field = velocity_z
    boundary = '4'
    value_type = min
  []
  [min_z_velocity_side5]
    type = NekSideExtremeValue
    field = velocity_z
    boundary = '5'
    value_type = min
  []
  [min_z_velocity_side6]
    type = NekSideExtremeValue
    field = velocity_z
    boundary = '6'
    value_type = min
  []
  [min_z_velocity_side7]
    type = NekSideExtremeValue
    field = velocity_z
    boundary = '7'
    value_type = min
  []
  [min_z_velocity_side8]
    type = NekSideExtremeValue
    field = velocity_z
    boundary = '8'
    value_type = min
  []
[]
