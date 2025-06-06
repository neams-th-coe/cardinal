[Mesh]
  type = NekRSMesh
  boundary = '1'
  order = SECOND
[]

[Problem]
  type = NekRSProblem
  casename = 'lowMach'

  [FieldTransfers]
    [temp]
      type = NekFieldVariable
      direction = from_nek
      field = temperature
    []
    [P]
      type = NekFieldVariable
      direction = from_nek
      field = pressure
    []
    [vel_x]
      type = NekFieldVariable
      direction = from_nek
      field = velocity_x
    []
    [vel_y]
      type = NekFieldVariable
      direction = from_nek
      field = velocity_y
    []
    [vel_z]
      type = NekFieldVariable
      direction = from_nek
      field = velocity_z
    []
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [max_T_side]
    type = NekSideExtremeValue
    field = temperature
    value_type = max
    boundary = '1'
  []
  [max_T_side_output]
    type = NodalExtremeValue
    variable = temp
    value_type = max
  []
  [max_T_side_diff]
    type = DifferencePostprocessor
    value1 = max_T_side
    value2 = max_T_side_output
  []

  [min_T_side]
    type = NekSideExtremeValue
    field = temperature
    value_type = min
    boundary = '1'
  []
  [min_T_side_output]
    type = NodalExtremeValue
    variable = temp
    value_type = min
  []
  [min_T_side_diff]
    type = DifferencePostprocessor
    value1 = min_T_side
    value2 = min_T_side_output
  []

  [max_Vx_side]
    type = NekSideExtremeValue
    field = velocity_x
    value_type = max
    boundary = '1'
  []
  [max_Vx_side_output]
    type = NodalExtremeValue
    variable = vel_x
    value_type = max
  []
  [max_Vx_side_diff]
    type = DifferencePostprocessor
    value1 = max_Vx_side
    value2 = max_Vx_side_output
  []

  [min_Vx_side]
    type = NekSideExtremeValue
    field = velocity_x
    value_type = min
    boundary = '1'
  []
  [min_Vx_side_output]
    type = NodalExtremeValue
    variable = vel_x
    value_type = min
  []
  [min_Vx_side_diff]
    type = DifferencePostprocessor
    value1 = min_Vx_side
    value2 = min_Vx_side_output
  []

  [max_p_side]
    type = NekSideExtremeValue
    field = pressure
    value_type = max
    boundary = '1'
  []
  [max_p_side_output]
    type = NodalExtremeValue
    variable = P
    value_type = max
  []
  [min_p_side_diff]
    type = DifferencePostprocessor
    value1 = min_p_side
    value2 = min_p_side_output
  []

  [min_p_side]
    type = NekSideExtremeValue
    field = pressure
    value_type = min
    boundary = '1'
  []
  [min_p_side_output]
    type = NodalExtremeValue
    variable = P
    value_type = min
  []
  [max_p_side_diff]
    type = DifferencePostprocessor
    value1 = max_p_side
    value2 = max_p_side_output
  []

  [avg_T_side]
    type = NekSideAverage
    field = temperature
    boundary = '1'
  []
  [avg_T_side_output]
    type = ElementAverageValue
    variable = temp
  []
  [avg_T_side_diff]
    type = DifferencePostprocessor
    value1 = avg_T_side
    value2 = avg_T_side_output
  []

  [avg_Vx_side]
    type = NekSideAverage
    field = velocity_x
    boundary = '1'
  []
  [avg_Vx_side_output]
    type = ElementAverageValue
    variable = vel_x
  []
  [avg_Vx_side_diff]
    type = DifferencePostprocessor
    value1 = avg_Vx_side
    value2 = avg_Vx_side_output
  []

  [avg_p_side]
    type = NekSideAverage
    field = pressure
    boundary = '1'
  []
  [avg_p_side_output]
    type = ElementAverageValue
    variable = P
  []
  [avg_p_side_diff]
    type = DifferencePostprocessor
    value1 = avg_p_side
    value2 = avg_p_side_output
  []
[]

[Outputs]
  csv = true
  exodus = true
  execute_on = 'final'

  hide = 'max_Vx_side max_Vx_side_output max_p_side max_p_side_output min_Vx_side min_Vx_side_output min_p_side min_p_side_output avg_Vx_side avg_Vx_side_output avg_p_side avg_p_side_output max_T_side max_T_side_output min_T_side min_T_side_output avg_T_side avg_T_side_output'
[]
