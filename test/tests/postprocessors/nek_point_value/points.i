[Problem]
  type = NekRSProblem
  casename = 'brick'
  n_usrwrk_slots = 3

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
    expression = '(exp(x)+1)'
  []
  [usrwrk01]
    type = ParsedFunction
    expression = 'exp(y)+1'
  []
  [usrwrk02]
    type = ParsedFunction
    expression = 'exp(z)+1'
  []
  [f]
    type = ParsedFunction
    expression = 'x+y+z+t*100'
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

[Postprocessors]
  [vx]
    type = NekPointValue
    field = velocity_x
    point = '0.25 0.3 0.27'
  []
  [vy]
    type = NekPointValue
    field = velocity_y
    point = '0.25 0.3 0.27'
  []
  [vz]
    type = NekPointValue
    field = velocity_z
    point = '0.25 0.3 0.27'
  []
  [comp]
    type = NekPointValue
    field = velocity_component
    velocity_direction = '0.5 0.5 0.5'
    point = '0.25 0.3 0.27'
  []
  [vx2]
    type = NekPointValue
    field = velocity_x_squared
    point = '0.25 0.3 0.27'
  []
  [vy2]
    type = NekPointValue
    field = velocity_y_squared
    point = '0.25 0.3 0.27'
  []
  [vz2]
    type = NekPointValue
    field = velocity_z_squared
    point = '0.25 0.3 0.27'
  []
  [vel]
    type = NekPointValue
    field = velocity
    point = '0.25 0.3 0.27'
  []
  [temp]
    type = NekPointValue
    field = temperature
    point = '0.25 0.3 0.27'
  []
  [p]
    type = NekPointValue
    field = pressure
    point = '0.25 0.3 0.27'
  []
  [scalar01]
    type = NekPointValue
    field = scalar01
    point = '0.25 0.3 0.27'
  []
  [scalar02]
    type = NekPointValue
    field = scalar02
    point = '0.25 0.3 0.27'
  []
  [scalar03]
    type = NekPointValue
    field = scalar03
    point = '0.25 0.3 0.27'
  []
  [unity]
    type = NekPointValue
    field = unity
    point = '0.25 0.3 0.27'
  []
  [usrwrk00]
    type = NekPointValue
    field = usrwrk00
    point = '0.25 0.3 0.27'
  []
  [usrwrk01]
    type = NekPointValue
    field = usrwrk01
    point = '0.25 0.3 0.27'
  []
  [usrwrk02]
    type = NekPointValue
    field = usrwrk02
    point = '0.25 0.3 0.27'
  []

  # the value of _t used when evaluating the function will either be the time of the time step (timestep_begin)
  # or at the end of the timestep after time has already been incremented (timestep_end). Also, there is no
  # need to test all the fields individually, since the shift is applied after dimensionalizing (equivalent
  # operation for every field).
  [compf_end]
    type = NekPointValue
    field = velocity_component
    velocity_direction = '0.5 0.5 0.5'
    point = '0.25 0.3 0.27'
    function = f
    execute_on = timestep_end
  []
  [compf_begin]
    type = NekPointValue
    field = velocity_component
    velocity_direction = '0.5 0.5 0.5'
    point = '0.25 0.3 0.27'
    function = f
    execute_on = timestep_begin
  []
[]

[Outputs]
  csv = true
[]
