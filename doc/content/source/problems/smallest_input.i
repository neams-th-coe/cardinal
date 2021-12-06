[Problem]
  type = NekRSProblem
  casename = 'fluid'
[]

[Mesh]
  type = NekRSMesh
  boundary = '1 2'
  volume = true
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
