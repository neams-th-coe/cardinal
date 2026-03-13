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
  [tally_editor]
    type = OpenMCTallyEditor
    create_tally = true
    tally_id = 100
    scores = 'absorption'
    filter_ids = ''
    nuclides = 'U238'
  []
[]

[Executioner]
  type = Steady
[]
