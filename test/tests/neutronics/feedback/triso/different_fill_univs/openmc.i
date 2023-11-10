[Mesh]
  [solid]
    type = FileMeshGenerator
    file = ../solid.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  check_equal_mapped_tally_volumes = true

  # this input should error because the universes filling the tally cells
  # dont match identically
  identical_cell_fills = '2'
  check_identical_cell_fills = true

  power = 100.0
  scaling = 100.0
  temperature_blocks = '1 2'
  tally_blocks = '2'
  tally_type = cell
  cell_level = 1
[]

[Executioner]
  type = Transient
[]
