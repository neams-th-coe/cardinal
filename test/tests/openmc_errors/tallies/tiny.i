[Mesh]
  [box]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
    xmin = 0.0
    xmax = 1e-6
    ymin = 0.0
    ymax = 1e-6
    zmin = 0.0
    zmax = 1e-6
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  cell_level = 0

  [Tallies]
    [mesh]
      type = MeshTally
      check_tally_sum = false
    []
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [max_err]
    type = TallyRelativeError
  []
[]
