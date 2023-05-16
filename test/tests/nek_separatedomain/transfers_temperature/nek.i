[Problem]
  type = NekRSSeparateDomainProblem

  inlet_boundary  = '1'
  outlet_boundary = '2'

  casename = 'pyramid'
[]

[Mesh]
  type = NekRSMesh
  boundary = '1'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [inlet_V_max]
    type = NekSideExtremeValue
    boundary = '1'
    field = velocity
    value_type = max
  []
  [inlet_V_min]
    type = NekSideExtremeValue
    boundary = '1'
    field = velocity
    value_type = min
  []
  [outlet_V_max]
    type = NekSideExtremeValue
    boundary = '2'
    field = velocity
    value_type = max
  []
  [outlet_V_min]
    type = NekSideExtremeValue
    boundary = '2'
    field = velocity
    value_type = min
  []
  [inlet_T_max]
    type = NekSideExtremeValue
    boundary = '1'
    field = temperature
    value_type = max
  []
  [inlet_T_min]
    type = NekSideExtremeValue
    boundary = '1'
    field = temperature
    value_type = min
  []
  [outlet_T_max]
    type = NekSideExtremeValue
    boundary = '2'
    field = temperature
    value_type = max
  []
  [outlet_T_min]
    type = NekSideExtremeValue
    boundary = '2'
    field = temperature
    value_type = min
  []
  [P_check]
    type = ParsedPostprocessor
    function = 'inlet_P - outlet_P - dP'
    pp_names = 'inlet_P outlet_P dP'
  []
[]


[MultiApps]
  [sub]
    type = FullSolveMultiApp
    input_files = 'cardinal_sub.i'
    sub_cycling = true
  []
[]

[Transfers]
  [toNekRS_velocity_trans]
    type = MultiAppPostprocessorTransfer
    from_multi_app = sub
    reduction_type = average
    from_postprocessor = toNekRS_velocity
    to_postprocessor   = inlet_V
  []
  [toNekRS_temperature_trans]
    type = MultiAppPostprocessorTransfer
    from_multi_app = sub
    reduction_type = average
    from_postprocessor = toNekRS_temperature
    to_postprocessor   = inlet_T
  []
[]

[Outputs]
  csv = true
[]
