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

[Executioner]
  type = Steady
[]

[Postprocessors]
  [k]
    type = KEigenvalue
    value_type = 'combined'
  []
[]
