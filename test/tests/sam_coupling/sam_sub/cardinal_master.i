[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
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

[Outputs]
  exodus = true
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    app_type = SamApp
    input_files = 'sam_sub.i'
  []
[]
