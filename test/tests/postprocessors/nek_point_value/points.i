[Problem]
  type = NekRSProblem
  casename = 'brick'
  n_usrwrk_slots = 3

  # This is a hacky way to test that we can query data in usrwrk. Here, we write into
  # three different slots in usrwrk, and to ensure we effectively apply no normalization,
  # we set the initial source integral to be the volume integral of the requested
  # function. This should be improved in the future with a general transfer into usrwrk
  # of any data (not requiring normalization).
  [FieldTransfers]
    [usrwrk00]
      type = NekVolumetricSource
      initial_source_integral = 2.71828
      usrwrk_slot = 0
      direction = to_nek
    []
    [usrwrk01]
      type = NekVolumetricSource
      initial_source_integral = 2.71828
      usrwrk_slot = 1
      direction = to_nek
    []
    [usrwrk02]
      type = NekVolumetricSource
      initial_source_integral = 2.71828
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
    execute_on = initial
  []
  [usrwrk01]
    type = FunctionAux
    variable = usrwrk01
    function = usrwrk01
    execute_on = initial
  []
  [usrwrk02]
    type = FunctionAux
    variable = usrwrk02
    function = usrwrk02
    execute_on = initial
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
[]

[Outputs]
  csv = true
  hide = 'usrwrk00_integral usrwrk01_integral usrwrk02_integral'
[]
