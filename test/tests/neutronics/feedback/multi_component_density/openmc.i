[Mesh]
  [file]
    type = FileMeshGenerator
    file = ../../meshes/pincell.e
  []
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '1'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 500.0
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
