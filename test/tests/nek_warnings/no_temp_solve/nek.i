[Mesh]
  type = NekRSMesh
  boundary = '6'
[]

[Problem]
  type = NekRSProblem
  casename = 'brick'

  [FieldTransfers]
    [temp]
      type = NekFieldVariable
      field = temperature
      direction = from_nek
    []
  []
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
