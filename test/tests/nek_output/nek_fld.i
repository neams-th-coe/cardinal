[Problem]
  type = NekRSProblem
  casename = 'pyramid'
  output = 'temperature'
  n_usrwrk_slots = 2
  has_heat_source = false

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

[Outputs]
  exodus = true
[]
