[Problem]
  type = NekRSStandaloneProblem
  casename = 'brick'
[]

[Mesh]
  type = NekRSMesh
  exact = true
  boundary = '1 3'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  exodus = true
[]
