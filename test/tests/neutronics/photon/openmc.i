[Mesh]
  type = FileMesh
  file = ../meshes/pincell.e
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 500.0

  cell_level = 1

  [Tallies]
    [Cell]
      type = CellTally
      tally_blocks = '1'
      tally_score = 'heating'
      tally_estimator = tracklength
    []
  []
[]

[Executioner]
  type = Transient
[]
