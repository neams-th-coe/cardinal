[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSStandaloneProblem
  casename = 'lowMach'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [max_p]
    type = NekVolumeExtremeValue
    field = pressure
    value_type = max
  []
  [min_p]
    type = NekVolumeExtremeValue
    field = pressure
    value_type = min
  []
  [max_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = max
  []
  [min_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = min
  []
  [max_Vx]
    type = NekVolumeExtremeValue
    field = x_velocity
    value_type = max
  []
  [min_Vx]
    type = NekVolumeExtremeValue
    field = x_velocity
    value_type = min
  []
[]

[Outputs]
  csv = true
  execute_on = 'final'
[]
