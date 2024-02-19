[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
  [solid]
    type = CombinerGenerator
    inputs = sphere
    positions = '100 100 100'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 70.0
  tally_type = cell
  tally_blocks = '1'
  cell_level = 0
[]

[Executioner]
  type = Transient
[]
