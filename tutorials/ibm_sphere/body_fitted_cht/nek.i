[Mesh]
  type = NekRSMesh
  boundary = '4'
[]

[Problem]
  type = NekRSProblem
  casename = 'pebble'

  n_usrwrk_slots = 1

  [FieldTransfers]
    [flux]
      type = NekBoundaryFlux
      direction = to_nek
      usrwrk_slot = 0
    []
    [temperature]
      type = NekFieldVariable
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

#[Postprocessors]
#  [outlet_bulk_T]
#    type = NekMassFluxWeightedSideAverage
#    boundary = '2'
#    field = temperature
#  []
#  [inlet_bulk_T]
#    type = NekMassFluxWeightedSideAverage
#    boundary = '1'
#    field = temperature
#  []
#  [dT]
#    type = ParsedPostprocessor
#    expression = 'outlet_bulk_T - inlet_bulk_T'
#    pp_names = 'outlet_bulk_T inlet_bulk_T'
#  []
#[]

[Outputs]
  csv = true
  exodus = true
  hide = 'flux_integral'
[]
