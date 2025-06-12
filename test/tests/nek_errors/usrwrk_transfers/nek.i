[Mesh]
  type = NekRSMesh
  boundary = '6'
[]

[Problem]
  type = NekRSProblem
  casename = 'brick'

  [FieldTransfers]
    [flux]
      type = NekBoundaryFlux
      direction = to_nek
      usrwrk_slot = '1'
    []
    [source]
      type = NekBoundaryFlux
      direction = to_nek
      usrwrk_slot = '2'
    []
  []

  [ScalarTransfers]
    [scalar]
      type = NekScalarValue
      direction = to_nek
      usrwrk_slot = '3'
    []
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
