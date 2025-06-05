[Mesh]
  type = NekRSMesh
  volume = true
  displacements = 'dx dy dz'
[]

[AuxVariables]
  [dx]
  []
  [dy]
  []
  [dz]
  []
[]

[Problem]
  type = NekRSProblem
  casename = 'brick2'
  has_heat_source = false
  n_usrwrk_slots = 1

  [FieldTransfers]
    [temp]
      type = NekFieldVariable
      field = temperature
      direction = from_nek
    []
  []
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
