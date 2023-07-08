[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSStandaloneProblem
  casename = 'channel'
  output = 'pressure velocity'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [max_Vx]
    type = NekVolumeExtremeValue
    field = velocity_x
    value_type = max
  []
  [max_Vx_output]
    type = NodalExtremeValue
    variable = vel_x
    value_type = max
  []
  [max_Vx_diff]
    type = DifferencePostprocessor
    value1 = max_Vx
    value2 = max_Vx_output
  []

  [min_Vx]
    type = NekVolumeExtremeValue
    field = velocity_x
    value_type = min
  []
  [min_Vx_output]
    type = NodalExtremeValue
    variable = vel_x
    value_type = min
  []
  [min_Vx_diff]
    type = DifferencePostprocessor
    value1 = min_Vx
    value2 = min_Vx_output
  []

  [max_Vy]
    type = NekVolumeExtremeValue
    field = velocity_y
    value_type = max
  []
  [max_Vy_output]
    type = NodalExtremeValue
    variable = vel_y
    value_type = max
  []
  [max_Vy_diff]
    type = DifferencePostprocessor
    value1 = max_Vy
    value2 = max_Vy_output
  []

  [min_Vy]
    type = NekVolumeExtremeValue
    field = velocity_y
    value_type = min
  []
  [min_Vy_output]
    type = NodalExtremeValue
    variable = vel_y
    value_type = min
  []

  [max_p]
    type = NekVolumeExtremeValue
    field = pressure
    value_type = max
  []
  [max_p_output]
    type = NodalExtremeValue
    variable = P
    value_type = max
  []
  [max_p_diff]
    type = DifferencePostprocessor
    value1 = max_p
    value2 = max_p_output
  []

  [min_p]
    type = NekVolumeExtremeValue
    field = pressure
    value_type = min
  []
  [min_p_output]
    type = NodalExtremeValue
    variable = P
    value_type = min
  []
  [min_p_diff]
    type = DifferencePostprocessor
    value1 = min_p
    value2 = min_p_output
  []

  [area]
    type = NekSideIntegral
    field = unity
    boundary = '1'
  []
  [area_output]
    type = AreaPostprocessor
    boundary = '1'
  []
  [area_diff]
    type = DifferencePostprocessor
    value1 = area
    value2 = area_output
  []

  [volume]
    type = NekVolumeIntegral
    field = unity
  []
  [volume_output]
    type = VolumePostprocessor
  []
  [volume_diff]
    type = DifferencePostprocessor
    value1 = volume
    value2 = volume_output
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
    boundary = '1'
  []
  [max_Vx_side_diff]
    type = DifferencePostprocessor
    value1 = max_Vx_side
    value2 = max_Vx_side_output
  []

  [max_Vy_side]
    type = NekSideExtremeValue
    field = velocity_y
    value_type = max
    boundary = '1'
  []
  [max_Vy_side_output]
    type = NodalExtremeValue
    variable = vel_y
    value_type = max
    boundary = '1'
  []
  [max_Vy_side_diff]
    type = DifferencePostprocessor
    value1 = max_Vy_side
    value2 = max_Vy_side_output
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
    boundary = '1'
  []
  [min_Vx_side_diff]
    type = DifferencePostprocessor
    value1 = min_Vx_side
    value2 = min_Vx_side_output
  []

  [min_Vy_side]
    type = NekSideExtremeValue
    field = velocity_y
    value_type = min
    boundary = '1'
  []
  [min_Vy_side_output]
    type = NodalExtremeValue
    variable = vel_y
    value_type = min
    boundary = '1'
  []
  [min_Vy_side_diff]
    type = DifferencePostprocessor
    value1 = min_Vy_side
    value2 = min_Vy_side_output
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
    boundary = '1'
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
    boundary = '1'
  []
  [max_p_side_diff]
    type = DifferencePostprocessor
    value1 = max_p_side
    value2 = max_p_side_output
  []

  [avg_p]
    type = NekVolumeAverage
    field = pressure
  []
  [avg_p_output]
    type = ElementAverageValue
    variable = P
  []
  [avg_p_diff]
    type = DifferencePostprocessor
    value1 = avg_p
    value2 = avg_p_output
  []

  [avg_Vx]
    type = NekVolumeAverage
    field = velocity_x
  []
  [avg_Vx_output]
    type = ElementAverageValue
    variable = vel_x
  []
  [avg_Vx_diff]
    type = DifferencePostprocessor
    value1 = avg_Vx
    value2 = avg_Vx_output
  []

  [avg_Vy]
    type = NekVolumeAverage
    field = velocity_y
  []
  [avg_Vy_output]
    type = ElementAverageValue
    variable = vel_y
  []
  [avg_Vy_diff]
    type = DifferencePostprocessor
    value1 = avg_Vy
    value2 = avg_Vy_output
  []

  [avg_Vx_side]
    type = NekSideAverage
    field = velocity_x
    boundary = '1'
  []
  [avg_Vx_side_output]
    type = SideAverageValue
    variable = vel_x
    boundary = '1'
  []
  [avg_Vx_side_diff]
    type = DifferencePostprocessor
    value1 = avg_Vx_side
    value2 = avg_Vx_side_output
  []

  [avg_Vy_side]
    type = NekSideAverage
    field = velocity_y
    boundary = '1'
  []
  [avg_Vy_side_output]
    type = SideAverageValue
    variable = vel_y
    boundary = '1'
  []
  [avg_Vy_side_diff]
    type = DifferencePostprocessor
    value1 = avg_Vy_side
    value2 = avg_Vy_side_output
  []

  [avg_p_side]
    type = NekSideAverage
    field = pressure
    boundary = '1'
  []
  [avg_p_side_output]
    type = SideAverageValue
    variable = P
    boundary = '1'
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

  hide = 'max_Vx_output min_Vx_output max_Vy_output min_Vy_output max_p_output min_p_output area_output volume_output max_Vx_side max_Vx_side_output max_Vy_side max_Vy_side_output max_p_side max_p_side_output min_Vx_side min_Vx_side_output min_Vy_side min_Vy_side_output min_p_side min_p_side_output avg_p avg_p_output avg_Vx avg_Vx_output avg_Vy avg_Vy_output avg_Vx_side avg_Vx_side_output avg_Vy_side avg_Vy_side_output avg_p_side avg_p_side_output max_Vx max_Vy max_p min_Vx min_Vy min_p area'
[]
