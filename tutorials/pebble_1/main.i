# This file is solely for testing this tutorial, and is not needed to run NekRS
# as a standalone application.

[Mesh]
  [g]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    input_files = 'test.i'
    sub_cycling = true
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 1
[]
