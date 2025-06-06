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
    type = NekVolumeIntegral
    field = usrwrk00
  []
  [usrwrk01]
    type = NekVolumeIntegral
    field = usrwrk01
  []
  [usrwrk02]
    type = NekVolumeIntegral
    field = usrwrk02
  []
  [unity_int]
    type = NekVolumeIntegral
    field = unity
  []
  [temp_int]
    type = NekVolumeIntegral
    field = temperature
  []
  [s01_int]
    type = NekVolumeIntegral
    field = scalar01
  []
  [s02_int]
    type = NekVolumeIntegral
    field = scalar02
  []
  [s03_int]
    type = NekVolumeIntegral
    field = scalar03
  []
  [pressure_int]
    type = NekVolumeIntegral
    field = pressure
  []
  [velocity_int]
    type = NekVolumeIntegral
    field = velocity
  []
  [x_velocity_int]
    type = NekVolumeIntegral
    field = velocity_x
  []
  [y_velocity_int]
    type = NekVolumeIntegral
    field = velocity_y
  []
  [z_velocity_int]
    type = NekVolumeIntegral
    field = velocity_z
  []
  [x2_velocity_int]
    type = NekVolumeIntegral
    field = velocity_x_squared
  []
  [y2_velocity_int]
    type = NekVolumeIntegral
    field = velocity_y_squared
  []
  [z2_velocity_int]
    type = NekVolumeIntegral
    field = velocity_z_squared
  []
  [velocity_component]
    type = NekVolumeIntegral
    field = velocity_component
    velocity_direction = '0.1 0.2 -0.3'
  []
[]
