[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  reset_seed = true
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Postprocessors]
  [k]
    type = KEigenvalue
  []
[]

[Outputs]
  csv = true
[]
