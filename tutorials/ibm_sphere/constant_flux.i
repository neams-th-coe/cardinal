# this is the re-triangulated area
stl_area = 12.5249

[Mesh]
  type = NekRSMesh
  volume = true
  stl = "retriangulated.stl"
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
      initial_flux_integral_stl = ${fparse 1 * stl_area}
    []
  []

  # the heat flux is constant, so we really only need to send it into NekRS one
  # time; this will send every 100 time steps
  synchronization_interval = constant
  constant_interval = 100
[]

[AuxVariables]
  [rank]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [rank]
    type = ProcessorIDAux
    variable = rank
  []
[]

[ICs]
  [flux]
    type = ConstantIC
    variable = flux
    value = 1.0
    block = 9999
  []
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
  hide = 'inlet_bulk_T outlet_bulk_T'
  time_step_interval = 100
  exodus = true
[]
