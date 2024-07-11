[Mesh]
  [m]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -2
    xmax = 2
    ymin = -2
    ymax = 2
    zmin = -2
    zmax = 10
    nx = 5
    ny = 5
    nz = 5
  []
  [id]
    type = SubdomainIDGenerator
    input = m
    subdomain_id = 1
  []
[]

[ICs]
  [t]
    type = ConstantIC
    variable = temp
    value = 500.0
  []
  [rho]
    type = ConstantIC
    variable = density
    value = 1000.0
  []
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '1'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 10.0
  temperature_blocks = '1'
  density_blocks = '1'
  verbose = true

  cell_level = 0
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
[]
