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
  identical_tally_cell_fills = true
  check_identical_tally_cell_fills = true

  power = 100.0
  scaling = 100.0
  solid_blocks = '1 2'
  tally_blocks = '2'
  tally_type = cell
  solid_cell_level = 1
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
