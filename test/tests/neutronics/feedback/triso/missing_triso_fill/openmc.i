[Mesh]
  [solid]
    type = FileMeshGenerator
    file = ../solid.e
  []
  [change_block_ids_for_middle]
    type = SubdomainBoundingBoxGenerator
    input = solid
    bottom_left = '-1.0 -1.0 0.05'
    top_right = '1.0 1.0 0.07'
    block_id = '5'
  []
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '2'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 2000.0
  scaling = 100.0
  temperature_blocks = '2 5'
  cell_level = 1

  # This input should error because the offset we compute for shifting cell instances
  # in a TRISO universe cant account for the fact that we are skipping some of the
  # TRISO universes from the tally setup
  identical_cell_fills = '2'
  check_identical_cell_fills = true
[]

[Executioner]
  type = Transient
[]
