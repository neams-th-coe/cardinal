[Problem]
  type = NekRSProblem
  casename = 'brick'
  n_usrwrk_slots = 4

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
  [usrwrk]
    type = NekPointValue
    field = usrwrk02
    point = '0.25 0.3 0.27'
  []
[]
