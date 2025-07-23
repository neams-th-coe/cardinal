[Mesh]
  type = NekRSMesh
  order = SECOND
  volume = true
  parallel_type = replicated
[]

[Problem]
  type = NekRSProblem
  casename = 'elast_nomv'
  n_usrwrk_slots = 4

  [FieldTransfers]
    [disp]
      type = NekMeshDeformation
      direction = to_nek
      usrwrk_slot = '1 2 3'
    []
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
