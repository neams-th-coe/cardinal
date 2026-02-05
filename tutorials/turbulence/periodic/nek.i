[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = pipe

  [FieldTransfers]
    [temp]
      type = NekFieldVariable
      direction = from_nek
      field = temperature
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
  steady_state_detection = true
  steady_state_tolerance = 1e-3
  check_aux = true

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [avg_T]
    type = NekVolumeAverage
    field = temperature
  []
  [max_T]
    type = NekVolumeExtremeValue
    field = temperature
  []
  [min_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = min
  []
[]
