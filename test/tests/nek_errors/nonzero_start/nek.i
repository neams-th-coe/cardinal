[Mesh]
  type = NekRSMesh
  boundary = '3'
[]

[Problem]
  type = NekRSProblem
  casename = 'brick'
[]

[Executioner]
  type = Transient
  start_time = 2

  [TimeStepper]
    type = NekTimeStepper
  []
[]
