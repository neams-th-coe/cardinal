[Mesh]
  type = NekRSMesh
  volume = true
  boundary = '2'
  parallel_type = replicated
[]

[Problem]
  type = NekRSProblem
  casename = 'user'
  has_heat_source = false
[]

[Executioner]
  type = Transient
  [TimeStepper]
    type = NekTimeStepper
  []
[]
