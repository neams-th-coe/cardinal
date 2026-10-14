[Mesh]
  type = NekRSMesh
  boundary = '1'
[]

[Problem]
  type = NekRSProblem
  casename = 'cube'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
