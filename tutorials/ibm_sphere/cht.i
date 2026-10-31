[Mesh]
  type = NekRSMesh
  volume = true
  stl = "small_sphere.stl"
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
  [source_flux_integral]
    type = ElementIntegralVariablePostprocessor
    variable = flux
    block = 9999
    execute_on = transfer
  []
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
  hide = 'inlet_bulk_T outlet_bulk_T'
  time_step_interval = 100
  exodus = true
[]
