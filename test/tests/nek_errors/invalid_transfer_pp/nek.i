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
  [synchronize_in]
    type = Receiver
  []
[]

[Outputs]
  exodus = true
  execute_on = 'final'
[]
