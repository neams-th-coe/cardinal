[Problem]
  type = NekRSProblem
  casename = 'pyramid'
  n_usrwrk_slots = 4

  [FieldTransfers]
    [usrwrk00]
      type = NekVolumetricData
      usrwrk_slot = 0
      direction = to_nek
    []
    [usrwrk01]
      type = NekVolumetricData
      usrwrk_slot = 1
      direction = to_nek
    []
    [usrwrk02]
      type = NekVolumetricData
      usrwrk_slot = 2
      direction = to_nek
    []
  []
[]

[AuxKernels]
  [usrwrk00]
    type = FunctionAux
    variable = usrwrk00
    function = usrwrk00
    execute_on = timestep_begin
  []
  [usrwrk01]
    type = FunctionAux
    variable = usrwrk01
    function = usrwrk01
    execute_on = timestep_begin
  []
  [usrwrk02]
    type = FunctionAux
    variable = usrwrk02
    function = usrwrk02
    execute_on = timestep_begin
  []
[]

[Functions]
  [usrwrk00]
    type = ParsedFunction
    expression = 'exp(x)+1'
  []
  [usrwrk01]
    type = ParsedFunction
    expression = 'exp(y)+1'
  []
  [usrwrk02]
    type = ParsedFunction
    expression = 'exp(z)+1'
  []
[]

[Mesh]
  type = NekRSMesh
  volume = true
  exact = true
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
