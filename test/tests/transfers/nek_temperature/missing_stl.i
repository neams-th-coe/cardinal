[Problem]
  type = NekRSProblem
  casename = 'pyramid'

  [FieldTransfers]
    [temperature]
      type = NekFieldVariable
      direction = from_nek
      mesh = stl
    []
  []
[]

[Mesh]
  type = NekRSMesh
  volume = true
  # this file is missing an STL surface, but yet we refer to that surface in the FieldTransfers - error
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
