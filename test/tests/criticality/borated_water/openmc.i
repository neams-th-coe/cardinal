[Mesh]
  [g]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 0
    xmax = 10
    ymin = 0
    ymax = 10
    zmin = 0
    zmax = 10
  []
[]

[Problem]
  type = OpenMCCellAverageProblem

  [CriticalitySearch]
    type = BoratedWater
    material_id = 2
    minimum = 100
    maximum = 3000
    tolerance = 1e-2
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [k]
    type = KEigenvalue
  []
[]

[Outputs]
  csv = true
[]
