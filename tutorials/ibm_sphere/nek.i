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

[Postprocessors]
  [average_inlet_v]
    type = NekSideAverage
    boundary = '1'
    field = velocity_z
  []
  [outlet_bulk_T]
    type = NekMassFluxWeightedSideIntegral
    boundary = '2'
    field = temperature
  []
  [inlet_bulk_T]
    type = NekMassFluxWeightedSideIntegral
    boundary = '1'
    field = temperature
  []
  [inlet_mdot]
    type= NekMassFluxWeightedSideIntegral
    boundary = '1'
    field = unity
  []
  [outlet_mdot]
    type= NekMassFluxWeightedSideIntegral
    boundary = '2'
    field = unity
  []
[]
