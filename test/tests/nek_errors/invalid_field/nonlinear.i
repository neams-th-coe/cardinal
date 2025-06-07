[Problem]
  type = NekRSProblem
  casename = 'pyramid'

  [FieldTransfers]
    [temp]
      type = NekFieldVariable
      direction = from_nek
      field = temperature
    []
  []
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
