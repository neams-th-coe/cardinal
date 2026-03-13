[Mesh]
  [initial]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -25.0
    xmax = 25.0
    ymin = -25.0
    ymax = 25.0
    zmin = -50.0
    zmax = 50.0
    nx = 1
    ny = 1
    nz = 10
  []
  [split]
    type = ParsedSubdomainMeshGenerator
    input = initial
    combinatorial_geometry = 'z < 0.0'
    block_id = '3'
  []
[]

[ICs]
  [density_1]
    type = FunctionIC
    variable = rho_1
    function = density
    block = '0'
  []
  [density_2]
    type = FunctionIC
    variable = rho_2
    function = density
    block = '3'
  []
[]

# Left side has a density of 500, right side has a density of 1000
[Functions]
  [density]
    type = ParsedFunction
    expression = '500.0 + 500.0 * (z + 50.0) / 100.0'
  []
[]

[AuxVariables]
  [cell_density]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_density]
    type = CellDensityAux
    variable = cell_density
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  cell_level = 0
  density_blocks = '0; 3'
  density_variables = 'rho_1; rho_2'
  mgxs_reference_densities = '1000.0 500.0'

  batches = 100
  inactive_batches = 10
  particles = 1000
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [k]
    type = KEigenvalue
    value_type = 'tracklength'
  []
  [rho_left]
    type = PointValue
    variable = cell_density
    point = '0.0 0.0 -25.0'
  []
  [rho_right]
    type = PointValue
    variable = cell_density
    point = '0.0 0.0 25.0'
  []
[]

[Outputs]
  csv = true
[]
