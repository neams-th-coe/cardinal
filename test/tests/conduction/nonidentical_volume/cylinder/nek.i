[Problem]
  type = NekRSProblem
[]

[Mesh]
  type = NekRSMesh
  order = SECOND
  volume = true
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [source_integral]
    type = Receiver
  []
  [max_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = max
  []
  [avg_T_volume]
    type = NekVolumeAverage
    field = temperature
  []
[]

[Outputs]
  exodus = true
  execute_on = 'final'
[]
