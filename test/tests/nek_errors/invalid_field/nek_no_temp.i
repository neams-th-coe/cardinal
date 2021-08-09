[Problem]
  type = NekRSStandaloneProblem
  casename = 'pyramid'
  output = 'temperature'
[]

[Mesh]
  type = NekRSMesh
  volume = true
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
