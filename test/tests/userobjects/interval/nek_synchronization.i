[Problem]
  type = NekRSStandaloneProblem
  casename = 'brick'
  output = 'velocity'

  nondimensional = true
  L_ref = 2.0
  U_ref = 1.0
  rho_0 = 834.5
  Cp_0 = 1228.0
  T_ref = 573.0
  dT_ref = 10.0

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
