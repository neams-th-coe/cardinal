[Mesh]
  [file]
    type = FileMeshGenerator
    file = ../dagmc/mesh_tallies/slab.e
  []
  [split_right]
    type = ParsedSubdomainMeshGenerator
    input = file
    combinatorial_geometry = 'x > 37.5'
    block_id = 100
    excluded_subdomains = '1'
  []
  [split_left]
    type = ParsedSubdomainMeshGenerator
    input = split_right
    combinatorial_geometry = 'x < 37.5'
    block_id = 200
    excluded_subdomains = '2'
  []
  [scale]
    type = TransformGenerator
    input = split_left
    transform = translate
    vector_value = '0.0 0.0 0.0'
  []
[]

[AuxVariables]
  [cell_temperature]
    family = MONOMIAL
    order = CONSTANT
  []
  [cell_density]
    family = MONOMIAL
    order = CONSTANT
  []
  [cell_material]
    family = MONOMIAL
    order = CONSTANT
  []
[]

x0 = 12.5
x1 = 37.5
x2 = 62.5

T0 = 600.0
dT = 50.0

drho = 100.0
rho0 = 1000.0

[Functions]
  [temp]
    type = ParsedFunction
    expression = 'if (x <= ${x0}, ${fparse T0 - dT}, if (x <= ${x1}, ${T0}, if (x <= ${x2}, ${fparse T0 + dT}, ${fparse T0 + 2 * dT})))'
  []
  [rho]
    type = ParsedFunction
    expression = 'if (x <= ${x0}, ${fparse rho0 - drho}, if (x <= ${x1}, ${rho0}, if (x <= ${x2}, ${fparse rho0 + drho}, ${fparse rho0 + 2 * drho})))'
  []
[]

[AuxKernels]
  [cell_temperature]
    type = CellTemperatureAux
    variable = cell_temperature
  []
  [cell_density]
    type = CellDensityAux
    variable = cell_density
  []
  [cell_material]
    type = CellMaterialIDAux
    variable = cell_material
  []
[]

[AuxKernels]
  [temp]
    type = FunctionAux
    variable = temp
    function = temp
    execute_on = timestep_begin
  []
  [rho]
    type = FunctionAux
    variable = density
    function = rho
    execute_on = timestep_begin
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  density_blocks = '2 1'
  temperature_blocks = '200 1'
  cell_level = 0
[]

[Postprocessors]
  [k]
    type = KEigenvalue
  []
  [rho_2]
    type = ElementAverageValue
    variable = density
    block = '2'
  []
  [rho_1]
    type = ElementAverageValue
    variable = density
    block = '1'
  []
  [T_200]
    type = ElementAverageValue
    variable = temp
    block = '200'
  []
  [T_1]
    type = ElementAverageValue
    variable = temp
    block = '1'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
