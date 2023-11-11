[Mesh]
  [file]
    type = FileMeshGenerator
    file = ../../meshes/pincell.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 500.0
  tally_blocks = '1'
  tally_type = cell
  cell_level = 1
  verbose = true

  density_variables = 'density1'
  density_blocks = '1; 2 3'
  temperature_blocks = '1 2 3'
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
