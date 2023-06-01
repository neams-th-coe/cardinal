[Problem]
  type = NekRSProblem
  casename = 'cylinder'
  usrwrk_output = '1'
  usrwrk_output_prefix = 'src'
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
