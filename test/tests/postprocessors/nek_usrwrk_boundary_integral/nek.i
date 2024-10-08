[Problem]
  type = NekRSProblem
  casename = 'pyramid'
[]

[Mesh]
  type = NekRSMesh
  boundary = '1 2 3 4 5 6 7 8'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [usrwrk]
    type = NekUsrWrkBoundaryIntegral
    usrwrk_slot = 11
    boundary = '1'
  []
[]
