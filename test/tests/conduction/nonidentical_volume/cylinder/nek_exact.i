[Problem]
  type = NekRSProblem
  casename = 'cylinder_low'
  usrwrk_output = '1'
  usrwrk_output_prefix = 'src'

  [FieldTransfers]
    [heat_source]
      type = NekVolumetricSource
      usrwrk_slot = 0
      direction = to_nek
      postprocessor_to_conserve = source_integral
    []
    [temp]
      type = NekFieldVariable
      field = temperature
      direction = from_nek
    []
  []
[]

[Mesh]
  type = NekRSMesh
  volume = true
  exact = true
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [max_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = max
  []
  [avg_T_volume]
    type = NekVolumeAverage
    field = temperature
  []
  [point]
    type = PointValue
    variable = temp
    point = '0.5 0.0 0.0 '
  []
[]

[Outputs]
  exodus = true
  execute_on = 'final'
  csv = true
[]
