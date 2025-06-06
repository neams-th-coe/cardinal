[Problem]
  type = NekRSProblem
  casename = 'pyramid'
[]

[Mesh]
  type = NekRSMesh
  boundary = '1 2 3 4 5 6 7 8'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  csv = true
  execute_on = 'final'
[]

[Postprocessors]
  [max_temp]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = max
  []
  [min_temp]
    type = NekVolumeExtremeValue
    field = temperature
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
  [max_velocity]
    type = NekVolumeExtremeValue
    field = velocity
    value_type = max
  []
  [min_velocity]
    type = NekVolumeExtremeValue
    field = velocity
    value_type = min
  []
  [max_x_velocity]
    type = NekVolumeExtremeValue
    field = velocity_x
    value_type = max
  []
  [min_x_velocity]
    type = NekVolumeExtremeValue
    field = velocity_x
    value_type = min
  []
  [max_y_velocity]
    type = NekVolumeExtremeValue
    field = velocity_y
    value_type = max
  []
  [min_y_velocity]
    type = NekVolumeExtremeValue
    field = velocity_y
    value_type = min
  []
  [max_z_velocity]
    type = NekVolumeExtremeValue
    field = velocity_z
    value_type = max
  []
  [min_z_velocity]
    type = NekVolumeExtremeValue
    field = velocity_z
    value_type = min
  []
[]
