[Problem]
  type = NekRSProblem
  minimize_transfers_in = true
  transfer_in = synchronization_in
  minimize_transfers_out = true
[]

[Mesh]
  type = NekRSMesh
  boundary = '1'
[]

[Executioner]
  type = Transient
  timestep_tolerance = 1e-10

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [synchronization_in]
    type = Receiver
  []
  [flux_integral]
    type = Receiver
  []
  [nek_flux]
    type = NekHeatFluxIntegral
    boundary = '1'
  []
  [average_inlet_T]
    type = NekSideAverage
    field = temperature
    boundary = '3'
    execute_on = initial
  []
  [average_outlet_T]
    type = NekSideAverage
    field = temperature
    boundary = '4'
  []
  [dT]
    type = DifferencePostprocessor
    value1 = average_outlet_T
    value2 = average_inlet_T
  []
  [inlet_mdot]
    type = NekMassFluxWeightedSideIntegral
    field = unity
    boundary = '3'
    execute_on = initial
  []
[]

[Outputs]
  exodus = true
  execute_on = 'final'

  [screen]
    type = Console
    hide = 'average_inlet_T average_outlet_T synchronization_in'
  []
[]
