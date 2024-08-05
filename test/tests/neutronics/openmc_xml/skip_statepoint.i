[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0

  inactive_batches = 3
  batches = 8
  skip_statepoint = true
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
