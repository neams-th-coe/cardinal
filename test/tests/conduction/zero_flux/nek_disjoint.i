[Problem]
  type = NekRSProblem
  casename = 'pyramid'

  [FieldTransfers]
    [avg_flux]
      type = NekBoundaryFlux
      usrwrk_slot = 0
      direction = to_nek
      conserve_flux_by_sideset = true
      postprocessor_to_conserve = flux_integral
    []
    [temp]
      type = NekFieldVariable
      direction = from_nek
      field = temperature
    []
  []
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
