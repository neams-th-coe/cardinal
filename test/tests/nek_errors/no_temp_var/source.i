[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = 'brick'
  n_usrwrk_slots = 1

  [FieldTransfers]
    [heat]
      type = NekVolumetricSource
      direction = to_nek
      usrwrk_slot = 0
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
