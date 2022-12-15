[Mesh]
  [cube]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 40
    ny = 40
    nz = 20
    xmin = -15.0
    xmax = 15.0
    ymin = -15.0
    ymax = 15.0
    zmin = -10.0
    zmax = 10.0
  []
[]

[AuxVariables]
  [cell_id]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_id]
    type = CellIDAux
    variable = cell_id
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  tally_type = mesh
  solid_cell_level = 0
  solid_blocks = '0'
  check_tally_sum = false
  check_zero_tallies = false

  power = 1000.0
  particles = 20000
  initial_properties = xml
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
