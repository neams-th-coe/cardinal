[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = 'cube'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

stl_area = 12.5317

[Postprocessors]
  [average_inlet_v]
    type = NekSideAverage
    boundary = '1'
    field = velocity_z
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
  [inlet_area]
    type = NekSideIntegral
    boundary = '1'
    field = unity
  []
  [expected_dT]
    type = ParsedPostprocessor
    expression = '${stl_area}*1 / (average_inlet_v*inlet_area)'
    pp_names = 'average_inlet_v inlet_area'
  []
[]

[Outputs]
  csv = true
  hide = 'inlet_bulk_T outlet_bulk_T'
  time_step_interval = 100
[]
