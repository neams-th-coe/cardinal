[Problem]
  type = NekRSStandaloneProblem
  casename = 'brick'
  output = 'velocity'

  [Dimensionalize]
    L = 2.0
    U = 1.0
    rho = 834.5
    Cp = 1228.0
    T = 573.0
    dT = 10.0
  []

  synchronization_interval = constant
  constant_interval = 3
[]

[Mesh]
  type = NekRSMesh
  volume = true
  scaling = 2.0
[]

[Postprocessors]
  [vz_in]
    type = SideAverageValue
    variable = 'vel_z'
    boundary = '1'
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  csv = true
[]
