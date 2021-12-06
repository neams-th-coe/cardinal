[Problem]
  type = NekRSProblem
  casename = 'cylinder'
  minimize_transfers_in = true
  minimize_transfers_out = true
[]

[Mesh]
  type = NekRSMesh
  boundary = '2'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [max_temp_nek]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = max
  []
  [min_temp_nek]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = min
  []
[]

[Outputs]
  exodus = true
  execute_on = 'final'
[]
