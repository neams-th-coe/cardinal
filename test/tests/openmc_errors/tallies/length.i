[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  tally_type = cell
  tally_blocks = '1'
  cell_level = 0
[]

[Executioner]
  type = Transient
[]
