[Mesh]
 [3d]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 2
    xmin = -12.5
    xmax = 87.5
    ymin = -12.5
    ymax = 37.5
    zmin = -12.5
    zmax = 12.5
  []
  [upper_block]
    type = ParsedSubdomainMeshGenerator
    input = 3d
    combinatorial_geometry = 'y > 12.5'
    block_id = 1
  []
  [split_top]
    type = ParsedSubdomainMeshGenerator
    input = upper_block
    combinatorial_geometry = 'x > 30.0 & y > 12.5'
    block_id = 50
  []
[]

[AuxVariables]
  [cell_density]
    family = MONOMIAL
    order = CONSTANT
  []
  [total_density]
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

[ICs]
  [d1]
    type = ConstantIC
    variable = density1
    value = 15e3
    block = '0'
  []
  [d2]
    type = ConstantIC
    variable = density2
    value = 3000
    block = '1'
  []
  [d3]
    type = ConstantIC
    variable = density2
    value = 7000
    block = '50'
  []
  [dd1]
    type = ConstantIC
    variable = total_density
    value = 15e3
    block = '0'
  []
  [dd2]
    type = ConstantIC
    variable = total_density
    value = 3000
    block = '1'
  []
  [dd3]
    type = ConstantIC
    variable = total_density
    value = 7000
    block = '50'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  cell_level = 0

  density_variables = 'density1; density2'
  density_blocks = '0; 1 50'
  temperature_blocks = '0 1 50'
[]

[Postprocessors]
  [k]
    type = KEigenvalue
  []
  [rho_0]
    type = ElementAverageValue
    variable = cell_density
    block = '0'
  []
  [rho_1]
    type = ElementAverageValue
    variable = cell_density
    block = '1'
  []
  [avg_cell0]
    type = ElementAverageValue
    variable = cell_density
    block = '0'
  []
  [avg_cell1]
    type = ElementAverageValue
    variable = cell_density
    block = '1 50'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  csv = true
  exodus = true
[]
