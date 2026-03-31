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
