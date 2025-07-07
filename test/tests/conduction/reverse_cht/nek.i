[Problem]
  type = NekRSProblem
  casename = 'cube'

  [FieldTransfers]
    [flux]
      type = NekBoundaryFlux
      direction = from_nek
    []
    [temperature]
      type = NekFieldVariable
      direction = to_nek
      usrwrk_slot = 1
    []
  []
[]

[Mesh]
  type = NekRSMesh
  boundary = '4'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [interface_T]
    type = NekSideAverage
    field = temperature
    boundary = '4'
  []
[]

[Outputs]
  exodus = true
[]
