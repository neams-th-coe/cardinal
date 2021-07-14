[Problem]
  type = NekRSProblem
[]

[Mesh]
  type = NekRSMesh
  boundary = '1 2 3 4 5 6 7 8'
  fixed_mesh = true
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  [out]
    type = CSV
    hide = 'flux_integral'
    execute_on = 'final'
  []
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
    field = x_velocity
    boundary = '1'
  []
  [x_velocity_avg2]
    type = NekSideAverage
    field = x_velocity
    boundary = '2'
  []
  [x_velocity_avg3]
    type = NekSideAverage
    field = x_velocity
    boundary = '3'
  []
  [x_velocity_avg4]
    type = NekSideAverage
    field = x_velocity
    boundary = '4'
  []
  [x_velocity_avg5]
    type = NekSideAverage
    field = x_velocity
    boundary = '5'
  []
  [x_velocity_avg6]
    type = NekSideAverage
    field = x_velocity
    boundary = '6'
  []
  [x_velocity_avg7]
    type = NekSideAverage
    field = x_velocity
    boundary = '7'
  []
  [x_velocity_avg8]
    type = NekSideAverage
    field = x_velocity
    boundary = '8'
  []
  [y_velocity_avg1]
    type = NekSideAverage
    field = y_velocity
    boundary = '1'
  []
  [y_velocity_avg2]
    type = NekSideAverage
    field = y_velocity
    boundary = '2'
  []
  [y_velocity_avg3]
    type = NekSideAverage
    field = y_velocity
    boundary = '3'
  []
  [y_velocity_avg4]
    type = NekSideAverage
    field = y_velocity
    boundary = '4'
  []
  [y_velocity_avg5]
    type = NekSideAverage
    field = y_velocity
    boundary = '5'
  []
  [y_velocity_avg6]
    type = NekSideAverage
    field = y_velocity
    boundary = '6'
  []
  [y_velocity_avg7]
    type = NekSideAverage
    field = y_velocity
    boundary = '7'
  []
  [y_velocity_avg8]
    type = NekSideAverage
    field = y_velocity
    boundary = '8'
  []
  [z_velocity_avg1]
    type = NekSideAverage
    field = z_velocity
    boundary = '1'
  []
  [z_velocity_avg2]
    type = NekSideAverage
    field = z_velocity
    boundary = '2'
  []
  [z_velocity_avg3]
    type = NekSideAverage
    field = z_velocity
    boundary = '3'
  []
  [z_velocity_avg4]
    type = NekSideAverage
    field = z_velocity
    boundary = '4'
  []
  [z_velocity_avg5]
    type = NekSideAverage
    field = z_velocity
    boundary = '5'
  []
  [z_velocity_avg6]
    type = NekSideAverage
    field = z_velocity
    boundary = '6'
  []
  [z_velocity_avg7]
    type = NekSideAverage
    field = z_velocity
    boundary = '7'
  []
  [z_velocity_avg8]
    type = NekSideAverage
    field = z_velocity
    boundary = '8'
  []
[]
