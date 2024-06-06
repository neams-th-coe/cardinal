[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
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
  [grid]
    type = CartesianGrid
    variable = g
    nx = 4
    ny = 4
    nz = 4
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]
