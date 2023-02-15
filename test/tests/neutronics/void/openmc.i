[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [multiple]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 8'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 10.0
  tally_type = 'cell'
  solid_cell_level = 0
  solid_blocks = '1'

  initial_properties = xml
[]

[Executioner]
  type = Steady
[]
