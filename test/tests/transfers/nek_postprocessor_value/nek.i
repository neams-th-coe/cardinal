[Problem]
  type = NekRSProblem
  casename = 'pyramid'
  n_usrwrk_slots = 2

  [ScalarTransfers]
    [inlet_V]
      type = NekPostprocessorValue
      direction = to_nek
      usrwrk_slot = 0
    []
    [inlet_T]
      type = NekPostprocessorValue
      direction = to_nek
      usrwrk_slot = 1
    []
  []
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
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    input_files = 'cardinal_sub.i'
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
