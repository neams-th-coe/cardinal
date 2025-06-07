[Problem]
  type = NekRSProblem
  synchronization_interval = parent_app
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
