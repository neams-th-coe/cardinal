[Mesh]
  type = NekRSMesh
  volume = true
  boundary = '2'
  parallel_type = replicated
[]

[Problem]
  type = NekRSProblem
  casename = 'user'

  [FieldTransfers]
    [disp]
      type = NekMeshDeformation
      direction = to_nek
      usrwrk_slot = '0 1 2'
    []
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
