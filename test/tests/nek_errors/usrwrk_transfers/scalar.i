[Mesh]
  type = NekRSMesh
  boundary = '6'
[]

[Problem]
  type = NekRSProblem
  casename = 'brick'
  n_usrwrk_slots = 5

  [ScalarTransfers]
    [scalar]
      type = NekScalarValue
      direction = to_nek
      usrwrk_slot = '3'
    []
  []

  [FieldTransfers]
    [flux]
      type = NekBoundaryFlux
      direction = to_nek
      usrwrk_slot = '3'
    []
    [flux2]
      type = NekBoundaryFlux
      direction = to_nek
      usrwrk_slot = '4'
    []
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
