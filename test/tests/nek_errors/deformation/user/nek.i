[Mesh]
  type = NekRSMesh
  volume = true
  boundary = '2'
  parallel_type = replicated
[]

[Problem]
  type = NekRSProblem
  casename = 'user'
  has_heat_source = false

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
