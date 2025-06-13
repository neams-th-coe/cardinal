[Problem]
  type = NekRSProblem
  casename = 'cylinder'

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
