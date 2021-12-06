[Problem]
  type = NekRSProblem
  minimize_transfers_in = true
  casename = 'cube'
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

[Outputs]
  exodus = true
  execute_on = 'final'
[]
