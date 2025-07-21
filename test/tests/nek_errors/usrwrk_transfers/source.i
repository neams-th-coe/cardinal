[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = 'brick'
  n_usrwrk_slots = 3

  [FieldTransfers]
    [source]
      type = NekVolumetricSource
      direction = to_nek
      usrwrk_slot = '1 2'
    []
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
