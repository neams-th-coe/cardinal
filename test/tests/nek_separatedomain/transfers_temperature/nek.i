[Problem]
  type = NekRSSeparateDomainProblem

  inlet_boundary  = '1'
  outlet_boundary = '2'

  casename = 'pyramid'
[]

[Mesh]
  type = NekRSMesh
  boundary = 1
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [inlet_V_check]
    type = NekSideAverage
    boundary = '1'
    field = velocity
  []
  [inlet_T_check]
    type = NekSideAverage
    boundary = '1'
    field = temperature
  []
  [outlet_V_check]
    type = NekSideAverage
    boundary = '2'
    field = velocity
  []
  [outlet_T_check]
    type = NekSideAverage
    boundary = '2'
    field = temperature
  []
[]


[MultiApps]
  [sub]
    type = FullSolveMultiApp
    app_type = CardinalApp
    input_files = 'cardinal_sub.i'
  []
[]

[Transfers]
  [toNekRS_velocity_trans]
    type = MultiAppPostprocessorTransfer
    multi_app = sub
    direction = from_multiapp
    reduction_type = average
    from_postprocessor = toNekRS_velocity
    to_postprocessor   = inlet_V
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [toNekRS_temperature_trans]
    type = MultiAppPostprocessorTransfer
    multi_app = sub
    direction = from_multiapp
    reduction_type = average
    from_postprocessor = toNekRS_temperature
    to_postprocessor   = inlet_T
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  csv = true
[]

