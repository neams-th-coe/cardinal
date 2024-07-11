[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../../neutronics/meshes/sphere.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
[]

[Executioner]
  type = Transient
[]
