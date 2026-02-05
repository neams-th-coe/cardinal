[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = pipe

  [FieldTransfers]
    [temp]
      type = NekFieldVariable
      direction = from_nek
      field = temperature
    []
    [vel_z]
      type = NekFieldVariable
      direction = from_nek
      field = velocity_z
    []
  []
[]

[Executioner]
  type = Transient
  steady_state_detection = true
  steady_state_tolerance = 1e-3
  check_aux = true

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  # this quantity will be 40 if we run to the steady-state
  [outlet_bulk_T]
    type = NekMassFluxWeightedSideAverage
    field = temperature
    boundary = '3'
  []
[]
