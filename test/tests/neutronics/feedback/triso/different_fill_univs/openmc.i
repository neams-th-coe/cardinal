[Mesh]
  [solid]
    type = FileMeshGenerator
    file = ../solid.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true

  # this input should error because the universes filling the tally cells
  # dont match identically
  identical_cell_fills = '2'
  check_identical_cell_fills = true

  power = 100.0
  scaling = 100.0
  temperature_blocks = '1 2'
  cell_level = 1

  [Tallies]
    [Cell]
      type = CellTally
      block = '2'
      check_equal_mapped_tally_volumes = true
    []
  []
[]

[Executioner]
  type = Transient
[]
