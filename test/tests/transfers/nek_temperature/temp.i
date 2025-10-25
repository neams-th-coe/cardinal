[Problem]
  type = NekRSProblem
  casename = 'pyramid'
  n_usrwrk_slots = 3

  [FieldTransfers]
    [T3]
      type = NekFieldVariable
      direction = to_nek
      usrwrk_slot = 2
      field = temperature
    []
    [T2]
      type = NekFieldVariable
      direction = to_nek
      usrwrk_slot = 1
      field = temperature
    []
  []
[]

[Mesh]
  type = NekRSMesh
  boundary = '1'
[]

[ICs]
  [T2]
    type = ConstantIC
    variable = T2
    value = 20
  []
  [T3]
    type = ConstantIC
    variable = T3
    value = 30
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [T3]
    type = NekUsrWrkBoundaryIntegral
    usrwrk_slot = 2
    boundary = '1'
  []
  [T2]
    type = NekUsrWrkBoundaryIntegral
    usrwrk_slot = 1
    boundary = '1'
  []
  [area]
    type = NekSideIntegral
    boundary = '1'
    field = unity
  []
[]

[Outputs]
  csv = true
[]
