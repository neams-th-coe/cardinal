[Problem]
  type = NekRSProblem
  minimize_transfers_in = true
  transfer_in = synchronization_in
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
  [flux_integral]
    type = Receiver
    default = 0
  []
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
  [synchronization_in]
    type = Receiver
  []
[]

[Outputs]
  exodus = true
  execute_on = 'final'
[]
