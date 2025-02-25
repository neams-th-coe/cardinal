[Mesh]
  [g]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Problem]
  type = FEProblem
  solve = false

  [Dimensionalize]
    U = 2.0
  []
[]

[Executioner]
  type = Steady
[]
