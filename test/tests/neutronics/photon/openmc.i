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
      block = '1'
      score = 'heating'
      estimator = tracklength
    []
  []
[]

[Executioner]
  type = Transient
[]
