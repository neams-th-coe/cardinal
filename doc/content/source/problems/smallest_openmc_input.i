[Mesh]
  type = FileMesh
  file = pincell.e
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 1000.0

  temperature_blocks = '1 2 3'
  density_blocks = '3'

  cell_level = 0

  [Tallies]
    [Cell]
      type = CellTally
      blocks = '1'
    []
  []
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
[]
