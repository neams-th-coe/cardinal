[Problem]
  type = NekRSStandaloneProblem
  casename = 'pyramid'
  output = 'temperature'
[]

[Mesh]
  type = NekRSMesh
  volume = true
[]

[Variables]
  [temp]
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
