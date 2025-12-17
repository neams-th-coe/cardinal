[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
[]

[Executioner]
  type = Transient
  num_steps = 1
  batches = 50 # this number needs to match the value in settings.xml for the purpose of the test
[]

[Postprocessors]
  [k]
    type = KEigenvalue
  []
[]

[Outputs]
  csv = true
[]
