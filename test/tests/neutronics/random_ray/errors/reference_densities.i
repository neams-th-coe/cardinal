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

  density_blocks = '0'
[]

[Executioner]
  type = Steady
[]
