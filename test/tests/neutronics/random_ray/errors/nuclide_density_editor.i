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
    normalize_by_global_tally = false
  []
[]

[UserObjects]
  [mat_editor]
    type = OpenMCNuclideDensities
    material_id = 1
    names = 'U235 U238'
    densities = '0.01 0.02'
  []
[]

[Executioner]
  type = Steady
[]
