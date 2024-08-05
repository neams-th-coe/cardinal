[Problem]
  type = NekRSProblem
  casename = pyramid
[]

[Mesh]
  type = NekRSMesh
  volume = true
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [temperature_average]
    type = NekVolumeAverage
    field = temperature
  []
  [pressure_average]
    type = NekVolumeAverage
    field = pressure
  []
  [velocity_average]
    type = NekVolumeAverage
    field = velocity
  []
  [x_velocity_average]
    type = NekVolumeAverage
    field = velocity_x
  []
  [y_velocity_average]
    type = NekVolumeAverage
    field = velocity_y
  []
  [z_velocity_average]
    type = NekVolumeAverage
    field = velocity_z
  []
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    input_files = 'cardinal_sub.i'
    execute_on = 'timestep_begin'
  []
[]
