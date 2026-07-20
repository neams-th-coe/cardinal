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
[]

[ICs]
  [density]
    type = FunctionIC
    variable = density
    function = density
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
  density_blocks = '0'
  mgxs_reference_densities_by_block = '1000.0'

  batches = 100
  inactive_batches = 10
  particles = 1000
[]

[Executioner]
  type = Steady
[]

