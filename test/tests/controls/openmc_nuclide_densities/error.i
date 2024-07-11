[Mesh]
  [g]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 8
    ny = 8
    nz = 8
    xmin = -12.5
    xmax = 87.5
    ymin = -12.5
    ymax = 37.5
    zmin = -12.5
    zmax = 12.5
  []
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = 0
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true

  temperature_blocks = '0'
  cell_level = 0
  power = 100.0
[]

[UserObjects]
  [mat1]
    type = OpenMCNuclideDensities
    material_id = 1
    names = 'U235'
    densities = '0.02'
  []
[]

[Controls]
  [c]
    type = OpenMCNuclideDensitiesControl
    user_object = none
    names = 'U235'
    densities = '0.01'
  []
[]

[Executioner]
  type = Steady
[]
