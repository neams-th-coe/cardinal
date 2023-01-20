[Mesh]
  type = NekRSMesh
  order = SECOND
  volume = true
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
