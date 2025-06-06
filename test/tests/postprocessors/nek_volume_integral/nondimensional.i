[Problem]
  type = NekRSProblem
  casename = 'pyramid'
  n_usrwrk_slots = 4

  [Dimensionalize]
    L = 2
    U = 0.2
    T = 10
    dT = 100
    s01 = 15
    ds01 = 150
    s02 = 20
    ds02 = 200
    s03 = 30
    ds03 = 300
    rho = 1000
    Cp = 4000
  []
[]

[Mesh]
  type = NekRSMesh
  volume = true
  scaling = 2
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
  [unity_int]
    type = NekVolumeIntegral
    field = unity
  []
  [temp_int]
    type = NekVolumeIntegral
    field = temperature
  []
  [u00_int]
    type = NekVolumeIntegral
    field = usrwrk00
  []
  [u01_int]
    type = NekVolumeIntegral
    field = usrwrk01
  []
  [u02_int]
    type = NekVolumeIntegral
    field = usrwrk02
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
