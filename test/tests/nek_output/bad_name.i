[Problem]
  type = NekRSProblem
  casename = 'pyramid'

  [FieldTransfers]
    [temp]
      type = NekFieldVariable
      direction = from_nek
    []
  []
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
