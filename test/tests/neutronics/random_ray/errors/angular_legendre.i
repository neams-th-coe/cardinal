[Mesh]
  [initial]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    zmin = 0
    zmax = 1
    nx = 1
    ny = 1
    nz = 1
  []
[]

[Problem]
  type = OpenMCCellAverageProblem

  power = 1
  cell_level = 0

  [Tallies/cell]
    type = CellTally
    filters = 'alf'
    normalize_by_global_tally = false
  []
  [Filters/alf]
    type = AngularLegendreFilter
    order = 1
  []
[]

[Executioner]
  type = Steady
[]
