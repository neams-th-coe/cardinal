[Mesh]
  type = NekRSMesh
  stl = "retriangulated.stl"

  # dummy, not used
  boundary = '1'
[]

[Problem]
  type = NekRSProblem
  casename = 'cube'

  n_usrwrk_slots = 1

  [FieldTransfers]
    [flux]
      type = NekBoundaryFlux
      mesh = 'stl'
      direction = to_nek
      usrwrk_slot = 0
    []
    [temperature]
      type = NekFieldVariable
      mesh = 'stl'
      direction = from_nek
    []
  []

  synchronization_interval = parent_app
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [outlet_bulk_T]
    type = NekMassFluxWeightedSideAverage
    boundary = '2'
    field = temperature
  []
  [inlet_bulk_T]
    type = NekMassFluxWeightedSideAverage
    boundary = '1'
    field = temperature
  []
  [dT]
    type = ParsedPostprocessor
    expression = 'outlet_bulk_T - inlet_bulk_T'
    pp_names = 'outlet_bulk_T inlet_bulk_T'
  []
[]

[Outputs]
  csv = true
  hide = 'inlet_bulk_T outlet_bulk_T flux_integral_stl dT'
  exodus = true
[]
