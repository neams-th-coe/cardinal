[Mesh]
  [initial]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = 1
    nx = 1
  []
[]

[Problem]
  type = OpenMCCellAverageProblem

  power = 1
  cell_level = 0

  [CriticalitySearch]
    type = BoratedWater
    absent_nuclides = 'O18'
    material_id = 2
    minimum = 1
    maximum = 2
    tolerance = 1e-2
  []
[]

[Executioner]
  type = Steady
[]
