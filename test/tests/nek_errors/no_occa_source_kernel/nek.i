[Problem]
  type = NekRSProblem
  casename = 'cube'
  n_usrwrk_slots = 2

  [FieldTransfers]
    [heat_source]
      type = NekVolumetricSource
      usrwrk_slot = 1
      direction = to_nek
    []
  []
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
