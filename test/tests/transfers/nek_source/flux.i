[Problem]
  type = NekRSProblem
  casename = 'pyramid'
  n_usrwrk_slots = 2

  [FieldTransfers]
    [src1]
      type = NekBoundaryFlux
      direction = to_nek
      usrwrk_slot = 1
      initial_flux_integral = 10
    []
    [src2]
      type = NekBoundaryFlux
      direction = to_nek
      usrwrk_slot = 0
      initial_flux_integral = 15
    []
  []
[]

[Mesh]
  type = NekRSMesh
  boundary = '2'
[]

[ICs]
  [src1]
    type = ConstantIC
    variable = src1
    value = 1
  []
  [src2]
    type = ConstantIC
    variable = src2
    value = 1
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [usrwrk1]
    type = NekUsrWrkBoundaryIntegral
    usrwrk_slot = 0
    boundary = '2'
  []
  [usrwrk2]
    type = NekUsrWrkBoundaryIntegral
    usrwrk_slot = 1
    boundary = '2'
  []
[]

[Outputs]
  csv = true
[]
