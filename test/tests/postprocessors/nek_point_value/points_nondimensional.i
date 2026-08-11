Lref = 5.0

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

  [Dimensionalize]
    L = ${Lref}
    U = 0.2
    T = 10.0
    dT = 200.0
    rho = 1000
    Cp = 3000

    s01 = 15.0
    ds01 = 250.0
    s02 = 20.0
    ds02 = 300.0
    s03 = 5.0
    ds03 = 100.0
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
    expression = 'exp(x/${Lref})+1'
  []
  [usrwrk01]
    type = ParsedFunction
    expression = 'exp(y/${Lref})+1'
  []
  [usrwrk02]
    type = ParsedFunction
    expression = 'exp(z/${Lref})+1'
  []
[]

[Mesh]
  type = NekRSMesh
  volume = true
  exact = true
  scaling = 5.0
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [temp]
    type = NekPointValue
    field = temperature
    point = '0.25 0.3 0.27'
  []
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
[]
