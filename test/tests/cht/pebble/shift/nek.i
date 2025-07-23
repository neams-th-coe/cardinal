[Mesh]
  type = NekRSMesh
  boundary = '1'
  scaling = 100.0
[]

[Problem]
  type = NekRSProblem
  casename = 'onepebble2'
  n_usrwrk_slots = 2

  [FieldTransfers]
    [flux]
      type = NekBoundaryFlux
      direction = to_nek
      usrwrk_slot = 1
    []
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
  csv = true
[]

[Postprocessors]
  [flux_in_nek]
    type = NekHeatFluxIntegral
    boundary = '1'
  []
  [max_nek_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = max
  []
  [min_nek_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = min
  []
[]
