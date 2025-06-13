[Mesh]
  [g]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 5
  []
[]

[Problem]
  type = FEProblem
  solve = false

  [ScalarTransfers]
    [scalar]
      type = NekScalarValue
      direction = to_nek
      usrwrk_slot = '3'
    []
  []
[]

[Executioner]
  type = Transient
[]
