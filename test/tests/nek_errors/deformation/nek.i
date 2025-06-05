[Mesh]
  type = NekRSMesh
  order = SECOND
  volume = true
  parallel_type = replicated
[]

[Problem]
  type = NekRSProblem
  has_heat_source = false
  casename = 'elast_nomv'

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
