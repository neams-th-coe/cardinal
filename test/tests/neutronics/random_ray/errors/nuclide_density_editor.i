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
