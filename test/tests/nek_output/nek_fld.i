[Problem]
  type = NekRSProblem
  casename = 'pyramid'
  n_usrwrk_slots = 2

  usrwrk_output = '2'
  usrwrk_output_prefix = 'ax'
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
