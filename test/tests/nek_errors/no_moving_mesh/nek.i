[Problem]
  type = NekRSProblem
  moving_mesh = true
  casename = 'cube'
[]

[Mesh]
  type = NekRSMesh
  volume = true
  parallel_type = replicated
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  exodus = true
[]
