[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Problem]
  type = NekRSProblem
  casename = 'brick'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
