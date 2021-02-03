[Problem]
  type = NekRSProblem
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
  []
[]

[Outputs]
  exodus = true
  execute_on = 'final'
[]
