[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSStandaloneProblem
  casename = 'conj_ht'
  output = 'temperature pressure scalar01'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [volume_nek_fluid]
    type = NekVolumeIntegral
    field = unity
    mesh = fluid
  []
  [volume_nek_solid]
    type = NekVolumeIntegral
    field = unity
    mesh = solid
  []
  [volume_nek_all]
    type = NekVolumeIntegral
    field = unity
    mesh = all
  []
  [volume_moose_fluid]
    type = VolumePostprocessor
    block = '0'
  []
  [volume_moose_solid]
    type = VolumePostprocessor
    block = '1'
  []
  [volume_moose_all]
    type = VolumePostprocessor
  []

  [int_T_nek_fluid]
    type = NekVolumeIntegral
    field = temperature
    mesh = fluid
  []
  [int_T_nek_solid]
    type = NekVolumeIntegral
    field = temperature
    mesh = solid
  []
  [int_T_nek_all]
    type = NekVolumeIntegral
    field = temperature
    mesh = all
  []
  [int_T_moose_fluid]
    type = ElementIntegralVariablePostprocessor
    variable = temp
    block = '0'
  []
  [int_T_moose_solid]
    type = ElementIntegralVariablePostprocessor
    variable = temp
    block = '1'
  []
  [int_T_moose_all]
    type = ElementIntegralVariablePostprocessor
    variable = temp
  []

  [int_p_nek_fluid]
    type = NekVolumeIntegral
    field = pressure
    mesh = fluid
  []
  [int_p_nek_solid]
    type = NekVolumeIntegral
    field = pressure
    mesh = solid
  []
  [int_p_nek_all]
    type = NekVolumeIntegral
    field = pressure
    mesh = all
  []
  [int_p_moose_fluid]
    type = ElementIntegralVariablePostprocessor
    variable =P
    block = '0'
  []
  [int_p_moose_solid]
    type = ElementIntegralVariablePostprocessor
    variable = P
    block = '1'
  []
  [int_p_moose_all]
    type = ElementIntegralVariablePostprocessor
    variable = P
  []

  [volume_avg_nek_fluid]
    type = NekVolumeAverage
    field = unity
    mesh = fluid
  []
  [volume_avg_nek_solid]
    type = NekVolumeAverage
    field = unity
    mesh = solid
  []
  [volume_avg_nek_all]
    type = NekVolumeAverage
    field = unity
    mesh = all
  []

  [avg_T_nek_fluid]
    type = NekVolumeAverage
    field = temperature
    mesh = fluid
  []
  [avg_T_nek_solid]
    type = NekVolumeAverage
    field = temperature
    mesh = solid
  []
  [avg_T_nek_all]
    type = NekVolumeAverage
    field = temperature
    mesh = all
  []
  [avg_T_moose_fluid]
    type = ElementAverageValue
    variable = temp
    block = '0'
  []
  [avg_T_moose_solid]
    type = ElementAverageValue
    variable = temp
    block = '1'
  []
  [avg_T_moose_all]
    type = ElementAverageValue
    variable = temp
  []

  [avg_p_nek_fluid]
    type = NekVolumeAverage
    field = pressure
    mesh = fluid
  []
  [avg_p_nek_solid]
    type = NekVolumeAverage
    field = pressure
    mesh = solid
  []
  [avg_p_nek_all]
    type = NekVolumeAverage
    field = pressure
    mesh = all
  []
  [avg_p_moose_fluid]
    type = ElementAverageValue
    variable = P
    block = '0'
  []
  [avg_p_moose_solid]
    type = ElementAverageValue
    variable = P
    block = '1'
  []
  [avg_p_moose_all]
    type = ElementAverageValue
    variable = P
  []

  [max_T_nek_fluid]
    type = NekVolumeExtremeValue
    field = temperature
    mesh = fluid
  []
  [max_T_nek_solid]
    type = NekVolumeExtremeValue
    field = temperature
    mesh = solid
  []
  [max_T_nek_all]
    type = NekVolumeExtremeValue
    field = temperature
    mesh = all
  []
  [max_T_moose_fluid]
    type = NodalExtremeValue
    variable = temp
    block = '0'
  []
  [max_T_moose_solid]
    type = NodalExtremeValue
    variable = temp
    block = '1'
  []
  [max_T_moose_all]
    type = NodalExtremeValue
    variable = temp
  []

  [min_T_nek_fluid]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = min
    mesh = fluid
  []
  [min_T_nek_solid]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = min
    mesh = solid
  []
  [min_T_nek_all]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = min
    mesh = all
  []
  [min_T_moose_fluid]
    type = NodalExtremeValue
    variable = temp
    value_type = min
    block = '0'
  []
  [min_T_moose_solid]
    type = NodalExtremeValue
    variable = temp
    value_type = min
    block = '1'
  []
  [min_T_moose_all]
    type = NodalExtremeValue
    variable = temp
    value_type = min
  []

  # we include these to show that the extrema postprocessors do properly fetch the true max/min
  # when the max/min is in either the solid or the fluid domain
  [min_q_nek_fluid]
    type = NekVolumeExtremeValue
    field = scalar01
    value_type = min
    mesh = fluid
  []
  [min_q_nek_solid]
    type = NekVolumeExtremeValue
    field = scalar01
    value_type = min
    mesh = solid
  []
  [min_q_nek_all]
    type = NekVolumeExtremeValue
    field = scalar01
    value_type = min
    mesh = all
  []
  [min_q_moose_fluid]
    type = NodalExtremeValue
    variable = scalar01
    value_type = min
    block = '0'
  []
  [min_q_moose_solid]
    type = NodalExtremeValue
    variable = scalar01
    value_type = min
    block = '1'
  []
  [min_q_moose_all]
    type = NodalExtremeValue
    variable = scalar01
    value_type = min
  []
[]

[Outputs]
  csv = true
  exodus = true
[]
