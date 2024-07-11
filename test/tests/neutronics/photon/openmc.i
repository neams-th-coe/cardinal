[Mesh]
  type = FileMesh
  file = ../meshes/pincell.e
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '1'
    tally_score = 'heating'
    tally_estimator = tracklength
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 500.0

  cell_level = 1
[]

[Executioner]
  type = Transient
[]
