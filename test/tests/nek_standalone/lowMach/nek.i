[Mesh]
  type = NekRSMesh
  volume = true
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
  # All the following postprocessors are applying operations both (a) directly to the NekRS
  # solution arrays, and (b) to the variables extracted with the 'outputs = ...' syntax.
  # Rather than check the actual values of these postprocessors (which might change if the
  # NekRS development team changes the nature of their CI tests), we can just check that
  # the difference between the Nek-style postprocessors from the MOOSE-style postprocessors
  # (acting on the extract solution) are nearly zero. We only check the absolute value of
  # the min/max volume values for Vx, temperature, and pressure because those values are printed to
  # the screen and offer quick confirmation of any changes that are due to changes in NekRS itself.

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

  [max_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = max
  []
  [max_T_output]
    type = NodalExtremeValue
    variable = temp
    value_type = max
  []
  [max_T_diff]
    type = DifferencePostprocessor
    value1 = max_T
    value2 = max_T_output
  []

  [min_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = min
  []
  [min_T_output]
    type = NodalExtremeValue
    variable = temp
    value_type = min
  []
  [min_T_diff]
    type = DifferencePostprocessor
    value1 = min_T
    value2 = min_T_output
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
    boundary = '1'
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
    boundary = '1'
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
    boundary = '1'
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
    boundary = '1'
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

  [avg_T]
    type = NekVolumeAverage
    field = temperature
  []
  [avg_T_output]
    type = ElementAverageValue
    variable = temp
  []
  [avg_T_diff]
    type = DifferencePostprocessor
    value1 = avg_T
    value2 = avg_T_output
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

  [avg_T_side]
    type = NekSideAverage
    field = temperature
    boundary = '1'
  []
  [avg_T_side_output]
    type = SideAverageValue
    variable = temp
    boundary = '1'
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
    type = SideAverageValue
    variable = vel_x
    boundary = '1'
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

  hide = 'max_Vx_output min_Vx_output max_p_output min_p_output area_output volume_output max_Vx_side max_Vx_side_output max_p_side max_p_side_output min_Vx_side min_Vx_side_output min_p_side min_p_side_output avg_Vx avg_Vx_output avg_Vx_side avg_Vx_side_output avg_p_side avg_p_side_output max_T_output min_T_output max_T_side max_T_side_output min_T_side min_T_side_output avg_T avg_T_output avg_T_side avg_T_side_output'
[]
