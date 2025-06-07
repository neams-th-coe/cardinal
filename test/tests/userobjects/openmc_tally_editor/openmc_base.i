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

  [Tallies]
    [Cell]
      type = CellTally
      block = '0'
    []
  []
[]

[Postprocessors]
  [power_1]
    type = PointValue
    variable = kappa_fission
    point = '0.0 -12.0 0.0'
  []
  [power_2]
    type = PointValue
    variable = kappa_fission
    point = '0.0 37.0 0.0'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
