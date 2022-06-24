[Mesh]
  type = NekRSMesh
  boundary = '6'
[]

[Problem]
  type = NekRSProblem
  casename = 'brick'

  n_usrwrk_slots = 0
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
