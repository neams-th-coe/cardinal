[Mesh]
  type = NekRSMesh
  volume = true
  scaling = 0.5
[]

[Problem]
  type = NekRSProblem
  casename = 'brick'

  [Dimensionalize]
    L = 2
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
