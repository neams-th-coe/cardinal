[Problem]
  type = NekRSProblem
  casename = 'pyramid'
  conserve_flux_by_sideset = true
[]

[Mesh]
  type = NekRSMesh
  boundary = '1 2'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  exodus = true
  csv = true
[]

[Postprocessors]
  [flux1]
    type = NekUsrWrkBoundaryIntegral
    usrwrk_slot = 0
    boundary = '1'
  []
  [flux2]
    type = NekUsrWrkBoundaryIntegral
    usrwrk_slot = 0
    boundary = '2'
  []
[]
