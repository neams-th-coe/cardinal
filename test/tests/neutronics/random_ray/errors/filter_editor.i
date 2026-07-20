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
  [filter_edit]
    type = OpenMCDomainFilterEditor
    create_filter = true
    filter_id = 100
    filter_type = 'cell'
    bins = '1'
  []
[]

[Executioner]
  type = Steady
[]
