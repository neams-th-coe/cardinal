[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  source_strength = 1e6
[]

[Executioner]
  type = Transient
[]

[Postprocessors]
  [k]
    type = KEigenvalue
  []
[]
