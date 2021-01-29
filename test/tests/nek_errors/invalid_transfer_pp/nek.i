[Problem]
  type = NekRSProblem
  minimize_transfers_in = true
  transfer_in = synchronize_in
[]

[Mesh]
  type = NekRSMesh
  boundary = '4'
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
    default = 0.0
  []
  [synchronize_in]
    type = Receiver
    default = 0.0
  []
[]

[Outputs]
  exodus = true
  execute_on = 'final'
[]
