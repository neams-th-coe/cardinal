[Mesh]
  type = NekRSMesh
  boundary = '2'
[]

[Problem]
  type = NekRSProblem
  casename = 'brick'

  [Dimensionalize]
    s01 = 0.5
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
