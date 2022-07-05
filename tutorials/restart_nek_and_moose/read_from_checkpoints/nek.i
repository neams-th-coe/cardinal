[Problem]
  type = NekRSProblem
  casename = pyramid
[]

[Mesh]
  type = NekRSMesh
  volume = true
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    app_type = CardinalApp
    input_files = 'cardinal_sub.i'
    execute_on = 'timestep_begin'
  []
[]
