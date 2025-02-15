[Problem]
  type = NekRSStandaloneProblem
  casename = 'brick'
  nondimensional = true
  L_ref = 5.0
  U_ref = 0.2
  T_ref = 10.0
  dT_ref = 200.0
  rho_0 = 1000
  Cp_0 = 3000
[]

[Mesh]
  type = NekRSMesh
  volume = true
  scaling = 5.0
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Functions]
  [s]
    type = ParsedFunction
    expression = 'sin(x)+sin(y)+z+10*t'
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
  [vc]
    type = NekPointValue
    field = velocity_component
    velocity_direction = '1 1 1'
    point = '0.25 0.3 0.27'
  []
  [vx_shift]
    type = NekPointValue
    field = velocity_x
    function = s
    point = '0.25 0.3 0.27'
  []
  [T_shift]
    type = NekPointValue
    field = temperature
    function = s
    point = '0.25 0.3 0.27'
  []
[]

[Outputs]
  csv = true
[]
