[Mesh]
  type = NekRSMesh
  volume = true
  order = SECOND
[]

[Problem]
  type = NekRSStandaloneProblem
  casename = 'ethier'
  output = 'temperature'
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
