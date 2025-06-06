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
  [temp_avg1]
    type = NekSideAverage
    field = temperature
    boundary = '1'
  []
  [temp_avg2]
    type = NekSideAverage
    field = temperature
    boundary = '2'
  []
  [temp_avg3]
    type = NekSideAverage
    field = temperature
    boundary = '3'
  []
  [temp_avg4]
    type = NekSideAverage
    field = temperature
    boundary = '4'
  []
  [temp_avg5]
    type = NekSideAverage
    field = temperature
    boundary = '5'
  []
  [temp_avg6]
    type = NekSideAverage
    field = temperature
    boundary = '6'
  []
  [temp_avg7]
    type = NekSideAverage
    field = temperature
    boundary = '7'
  []
  [temp_avg8]
    type = NekSideAverage
    field = temperature
    boundary = '8'
  []
  [pressure_avg1]
    type = NekSideAverage
    field = pressure
    boundary = '1'
  []
  [pressure_avg2]
    type = NekSideAverage
    field = pressure
    boundary = '2'
  []
  [pressure_avg3]
    type = NekSideAverage
    field = pressure
    boundary = '3'
  []
  [pressure_avg4]
    type = NekSideAverage
    field = pressure
    boundary = '4'
  []
  [pressure_avg5]
    type = NekSideAverage
    field = pressure
    boundary = '5'
  []
  [pressure_avg6]
    type = NekSideAverage
    field = pressure
    boundary = '6'
  []
  [pressure_avg7]
    type = NekSideAverage
    field = pressure
    boundary = '7'
  []
  [pressure_avg8]
    type = NekSideAverage
    field = pressure
    boundary = '8'
  []
  [velocity_avg1]
    type = NekSideAverage
    field = velocity
    boundary = '1'
  []
  [velocity_avg2]
    type = NekSideAverage
    field = velocity
    boundary = '2'
  []
  [velocity_avg3]
    type = NekSideAverage
    field = velocity
    boundary = '3'
  []
  [velocity_avg4]
    type = NekSideAverage
    field = velocity
    boundary = '4'
  []
  [velocity_avg5]
    type = NekSideAverage
    field = velocity
    boundary = '5'
  []
  [velocity_avg6]
    type = NekSideAverage
    field = velocity
    boundary = '6'
  []
  [velocity_avg7]
    type = NekSideAverage
    field = velocity
    boundary = '7'
  []
  [velocity_avg8]
    type = NekSideAverage
    field = velocity
    boundary = '8'
  []
  [x_velocity_avg1]
    type = NekSideAverage
    field = velocity_x
    boundary = '1'
  []
  [x_velocity_avg2]
    type = NekSideAverage
    field = velocity_x
    boundary = '2'
  []
  [x_velocity_avg3]
    type = NekSideAverage
    field = velocity_x
    boundary = '3'
  []
  [x_velocity_avg4]
    type = NekSideAverage
    field = velocity_x
    boundary = '4'
  []
  [x_velocity_avg5]
    type = NekSideAverage
    field = velocity_x
    boundary = '5'
  []
  [x_velocity_avg6]
    type = NekSideAverage
    field = velocity_x
    boundary = '6'
  []
  [x_velocity_avg7]
    type = NekSideAverage
    field = velocity_x
    boundary = '7'
  []
  [x_velocity_avg8]
    type = NekSideAverage
    field = velocity_x
    boundary = '8'
  []
  [y_velocity_avg1]
    type = NekSideAverage
    field = velocity_y
    boundary = '1'
  []
  [y_velocity_avg2]
    type = NekSideAverage
    field = velocity_y
    boundary = '2'
  []
  [y_velocity_avg3]
    type = NekSideAverage
    field = velocity_y
    boundary = '3'
  []
  [y_velocity_avg4]
    type = NekSideAverage
    field = velocity_y
    boundary = '4'
  []
  [y_velocity_avg5]
    type = NekSideAverage
    field = velocity_y
    boundary = '5'
  []
  [y_velocity_avg6]
    type = NekSideAverage
    field = velocity_y
    boundary = '6'
  []
  [y_velocity_avg7]
    type = NekSideAverage
    field = velocity_y
    boundary = '7'
  []
  [y_velocity_avg8]
    type = NekSideAverage
    field = velocity_y
    boundary = '8'
  []
  [z_velocity_avg1]
    type = NekSideAverage
    field = velocity_z
    boundary = '1'
  []
  [z_velocity_avg2]
    type = NekSideAverage
    field = velocity_z
    boundary = '2'
  []
  [z_velocity_avg3]
    type = NekSideAverage
    field = velocity_z
    boundary = '3'
  []
  [z_velocity_avg4]
    type = NekSideAverage
    field = velocity_z
    boundary = '4'
  []
  [z_velocity_avg5]
    type = NekSideAverage
    field = velocity_z
    boundary = '5'
  []
  [z_velocity_avg6]
    type = NekSideAverage
    field = velocity_z
    boundary = '6'
  []
  [z_velocity_avg7]
    type = NekSideAverage
    field = velocity_z
    boundary = '7'
  []
  [z_velocity_avg8]
    type = NekSideAverage
    field = velocity_z
    boundary = '8'
  []
  [velocity_comp1]
    type = NekSideIntegral
    field = velocity_component
    velocity_direction = '0.1 -0.2 0.3'
    boundary = '1'
  []
  [velocity_comp2]
    type = NekSideIntegral
    field = velocity_component
    velocity_direction = '0.1 -0.2 0.3'
    boundary = '2'
  []
  [velocity_comp3]
    type = NekSideIntegral
    field = velocity_component
    velocity_direction = '0.1 -0.2 0.3'
    boundary = '3'
  []
  [velocity_comp4]
    type = NekSideIntegral
    field = velocity_component
    velocity_direction = '0.1 -0.2 0.3'
    boundary = '4'
  []
  [velocity_comp5]
    type = NekSideIntegral
    field = velocity_component
    velocity_direction = '0.1 -0.2 0.3'
    boundary = '5'
  []
  [velocity_comp6]
    type = NekSideIntegral
    field = velocity_component
    velocity_direction = '0.1 -0.2 0.3'
    boundary = '6'
  []
  [velocity_comp7]
    type = NekSideIntegral
    field = velocity_component
    velocity_direction = '0.1 -0.2 0.3'
    boundary = '7'
  []
  [velocity_comp8]
    type = NekSideIntegral
    field = velocity_component
    velocity_direction = '0.1 -0.2 0.3'
    boundary = '8'
  []
  [x_velocity2_avg1]
    type = NekSideAverage
    field = velocity_x_squared
    boundary = '1'
  []
  [x_velocity2_avg2]
    type = NekSideAverage
    field = velocity_x_squared
    boundary = '2'
  []
  [x_velocity2_avg3]
    type = NekSideAverage
    field = velocity_x_squared
    boundary = '3'
  []
  [x_velocity2_avg4]
    type = NekSideAverage
    field = velocity_x_squared
    boundary = '4'
  []
  [x_velocity2_avg5]
    type = NekSideAverage
    field = velocity_x_squared
    boundary = '5'
  []
  [x_velocity2_avg6]
    type = NekSideAverage
    field = velocity_x_squared
    boundary = '6'
  []
  [x_velocity2_avg7]
    type = NekSideAverage
    field = velocity_x_squared
    boundary = '7'
  []
  [x_velocity2_avg8]
    type = NekSideAverage
    field = velocity_x_squared
    boundary = '8'
  []
  [y_velocity2_avg1]
    type = NekSideAverage
    field = velocity_y_squared
    boundary = '1'
  []
  [y_velocity2_avg2]
    type = NekSideAverage
    field = velocity_y_squared
    boundary = '2'
  []
  [y_velocity2_avg3]
    type = NekSideAverage
    field = velocity_y_squared
    boundary = '3'
  []
  [y_velocity2_avg4]
    type = NekSideAverage
    field = velocity_y_squared
    boundary = '4'
  []
  [y_velocity2_avg5]
    type = NekSideAverage
    field = velocity_y_squared
    boundary = '5'
  []
  [y_velocity2_avg6]
    type = NekSideAverage
    field = velocity_y_squared
    boundary = '6'
  []
  [y_velocity2_avg7]
    type = NekSideAverage
    field = velocity_y_squared
    boundary = '7'
  []
  [y_velocity2_avg8]
    type = NekSideAverage
    field = velocity_y_squared
    boundary = '8'
  []
  [z_velocity2_avg1]
    type = NekSideAverage
    field = velocity_z_squared
    boundary = '1'
  []
  [z_velocity2_avg2]
    type = NekSideAverage
    field = velocity_z_squared
    boundary = '2'
  []
  [z_velocity2_avg3]
    type = NekSideAverage
    field = velocity_z_squared
    boundary = '3'
  []
  [z_velocity2_avg4]
    type = NekSideAverage
    field = velocity_z_squared
    boundary = '4'
  []
  [z_velocity2_avg5]
    type = NekSideAverage
    field = velocity_z_squared
    boundary = '5'
  []
  [z_velocity2_avg6]
    type = NekSideAverage
    field = velocity_z_squared
    boundary = '6'
  []
  [z_velocity2_avg7]
    type = NekSideAverage
    field = velocity_z_squared
    boundary = '7'
  []
  [z_velocity2_avg8]
    type = NekSideAverage
    field = velocity_z_squared
    boundary = '8'
  []
[]
