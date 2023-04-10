[Mesh]
  type = GeneratedMesh
  nx = 8
  ny = 8
  nz = 8
  dim = 3
  xmin = -1.0
  xmax = 0.0
[]

[Problem]
  type = FEProblem
  solve = false
[]

[AuxVariables]
  [g]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [g]
    type = CartesianGrid
    variable = g
    nx = 4
    ny = 4
    nz = 4
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  execute_on = final
[]
