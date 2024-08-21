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

[Problem]
  type = OpenMCCellAverageProblem
  cell_level = 0
  power = 100.0

  check_tally_sum = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]

[UserObjects]
  [tally_editor_uo]
    type = OpenMCTallyEditor
    tally_id = 3
    nuclides = ''
    filter_ids = ''
    multiply_density = true
    scores = ''
  []
[]
