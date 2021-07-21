[Mesh]
  type = NekRSMesh
  order = FIRST
  volume = true
[]

[Problem]
  type = NekRSProblem
  moving_mesh = true
[]

[Executioner]
  type = Transient

  [./TimeStepper]
    type = NekTimeStepper
  [../]
[]

[Outputs]
  exodus = true
  execute_on = 'final'
  hide = 'heat_source'
[]

