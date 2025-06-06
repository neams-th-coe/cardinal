[Problem]
  type = NekRSProblem
  casename = 'pyramid'
  n_usrwrk_slots = 4
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
  csv = true
  execute_on = 'final'
[]

[Postprocessors]
  [usrwrk00]
    type = NekVolumeAverage
    field = usrwrk00
  []
  [usrwrk01]
    type = NekVolumeAverage
    field = usrwrk01
  []
  [usrwrk02]
    type = NekVolumeAverage
    field = usrwrk02
  []
  [unity_average]
    type = NekVolumeAverage
    field = unity
  []
  [temp_average]
    type = NekVolumeAverage
    field = temperature
  []
  [s01_average]
    type = NekVolumeAverage
    field = scalar01
  []
  [s02_average]
    type = NekVolumeAverage
    field = scalar02
  []
  [s03_average]
    type = NekVolumeAverage
    field = scalar03
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
  [x2_velocity_average]
    type = NekVolumeAverage
    field = velocity_x_squared
  []
  [y2_velocity_average]
    type = NekVolumeAverage
    field = velocity_y_squared
  []
  [z2_velocity_average]
    type = NekVolumeAverage
    field = velocity_z_squared
  []
  [velocity_component]
    type = NekVolumeAverage
    field = velocity_component
    velocity_direction = '0.1 0.2 -0.3'
  []
[]
