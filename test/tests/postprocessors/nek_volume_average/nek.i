[Problem]
  type = NekRSProblem
  casename = 'pyramid'
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

[Outputs]
  [out]
    type = CSV
    hide = 'source_integral'
    execute_on = 'final'
  []
[]

[Postprocessors]
  [temp_average]
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
  [velocity_component]
    type = NekVolumeAverage
    field = velocity_component
    velocity_direction = '0.1 0.2 -0.3'
  []
[]
