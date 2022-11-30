[Problem]
  type = NekRSStandaloneProblem
  casename = 'pyramid'
  output = 'temperature'
[]

[Mesh]
  type = NekRSMesh
  volume = true
[]

[AuxVariables]
  [temp]
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
