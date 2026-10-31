[Problem]
  type = NekRSProblem
  casename = 'brick'
  n_usrwrk_slots = 3
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

[Postprocessors]
  [stl_integral_0]
    type = NekUsrWrkSTLIntegral
    usrwrk_slot = 0
  []
[]
