[Mesh]
  type = NekRSMesh
  order = SECOND
  volume = true
  parallel_type = replicated
[]

[Problem]
  type = NekRSProblem
  casename = 'elast_nomv'

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
