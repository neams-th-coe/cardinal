[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  tally_type = none
  batches = 8
[]

[Executioner]
  type = Transient
[]
