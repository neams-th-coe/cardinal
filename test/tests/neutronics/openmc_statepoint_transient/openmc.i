[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  keep_transient_statepoint = true
[]

[Executioner]
  type = Transient
  num_steps = 2
[]
