[Mesh]
  type = NekRSMesh
  boundary = '1'
  order = SECOND
[]

[Problem]
  type = NekRSStandaloneProblem
  casename = 'mv_cyl'
  output = 'pressure velocity temperature'

  # We omit the non-dimensional settings here in order to just extract the
  # non-dimensional solution as-is, without dimensionalizing it.
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []

  [Quadrature]
    type = GAUSS_LOBATTO
    order = THIRD
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
[]

[Outputs]
  csv = true
  execute_on = 'final'
[]
