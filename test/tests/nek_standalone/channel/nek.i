[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSStandaloneProblem
  casename = 'channel'
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
    field = x_velocity
    value_type = max
  []
  [min_Vx]
    type = NekVolumeExtremeValue
    field = x_velocity
    value_type = min
  []
  [max_Vy]
    type = NekVolumeExtremeValue
    field = y_velocity
    value_type = max
  []
  [min_Vy]
    type = NekVolumeExtremeValue
    field = y_velocity
    value_type = min
  []
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
  [area]
    type = NekSideIntegral
    field = unity
    boundary = '1'
  []
  [volume]
    type = NekVolumeIntegral
    field = unity
  []
[]

[Outputs]
  csv = true
  execute_on = 'final'
[]
