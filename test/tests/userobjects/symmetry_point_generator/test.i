[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  type = FEProblem
  solve = false
[]

[UserObjects]
  [spg]
    type = SymmetryPointGenerator
    normal = '1.0 0.0 0.0'
  []
[]

[Executioner]
  type = Steady
[]
