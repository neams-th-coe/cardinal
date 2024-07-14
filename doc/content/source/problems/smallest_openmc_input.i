[Mesh]
  type = FileMesh
  file = pincell.e
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '1'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 1000.0

  temperature_blocks = '1 2 3'
  density_blocks = '3'

  cell_level = 0
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
[]
