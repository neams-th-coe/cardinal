[Problem]
  type = NekRSStandaloneProblem
  casename = 'brick'

  [Dimensionalize]
    L = 5.0
    U = 0.2
    T = 10.0
    dT = 200.0
    rho = 1000
    Cp = 3000
  []
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
[]

[Outputs]
  csv = true
[]
