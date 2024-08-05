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
[]

[AuxVariables]
  [cell_id]
    family = MONOMIAL
    order = CONSTANT
  []
  [cell_instance]
    family = MONOMIAL
    order = CONSTANT
  []
  [cell_temperature]
    family = MONOMIAL
    order = CONSTANT
  []
  [material_id]
    family = MONOMIAL
    order = CONSTANT
  []
  [cell_density]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_id]
    type = CellIDAux
    variable = cell_id
  []
  [cell_instance]
    type = CellInstanceAux
    variable = cell_instance
  []
  [cell_temperature]
    type = CellTemperatureAux
    variable = cell_temperature
  []
  [material_id]
    type = CellMaterialIDAux
    variable = material_id
  []
  [cell_density]
    type = CellDensityAux
    variable = cell_density
  []
[]

[ICs]
active = ''
  [temp]
    type = ConstantIC
    variable = temp
    value = 300.0
  []
  [density]
    type = ConstantIC
    variable = density
    value = 15.0e3
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
[]

[Postprocessors]
  [k]
    type = KEigenvalue
  []
  [id_0]
    type = ElementAverageValue
    variable = cell_id
    block = '0'
  []
  [id_1]
    type = ElementAverageValue
    variable = cell_id
    block = '1'
  []
  [instance_0]
    type = ElementAverageValue
    variable = cell_instance
    block = '0'
  []
  [instance_1]
    type = ElementAverageValue
    variable = cell_instance
    block = '1'
  []
  [temp_0]
    type = ElementAverageValue
    variable = cell_temperature
    block = '0'
  []
  [temp_1]
    type = ElementAverageValue
    variable = cell_temperature
    block = '1'
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
  [mat_0]
    type = ElementAverageValue
    variable = material_id
    block = '0'
  []
  [mat_1]
    type = ElementAverageValue
    variable = material_id
    block = '1'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  csv = true
[]
