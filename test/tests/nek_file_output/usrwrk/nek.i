[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = 'brick'
  usrwrk_output = '0 1'
  usrwrk_output_prefix = 'aaa ccc'
  n_usrwrk_slots = 2
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
