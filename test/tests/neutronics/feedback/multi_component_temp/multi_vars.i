[Mesh]
  [load]
    type = FileMeshGenerator
    file = ../../meshes/pincell.e
  []
  [split_fuel]
    type = ParsedSubdomainMeshGenerator
    combinatorial_geometry = 'z < 5.0'
    excluded_subdomains = '3 2'
    block_id = 10
    input = load
  []
[]

[AuxVariables]
  [cell_temp]
    family = MONOMIAL
    order = CONSTANT
  []
  [collated_temp]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_temp]
    type = CellTemperatureAux
    variable = cell_temp
  []
  [collated_solid_temp]
    type = ProjectionAux
    variable = collated_temp
    v = solid_temp
    block = '1 3'
    execute_on = timestep_begin
  []
  [collated_fluid_temp]
    type = ProjectionAux
    variable = collated_temp
    v = fluid_temp
    block = '2'
    execute_on = timestep_begin
  []
  [collated_other_temp]
    type = ProjectionAux
    variable = collated_temp
    v = other_temp
    block = '10'
    execute_on = timestep_begin
  []
[]

[ICs]
  [solid_temp]
    type = ConstantIC
    variable = solid_temp
    value = 800
  []
  [fluid_temp]
    type = ConstantIC
    variable = fluid_temp
    value = 600
  []
  [other_temp]
    type = ConstantIC
    variable = other_temp
    value = 400
  []
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '1 3 2 10'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 500.0
  cell_level = 0
  verbose = true

  temperature_variables = 'solid_temp; fluid_temp; other_temp'
  temperature_blocks = '1 3; 2; 10'
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [max_temp]
    type = ElementExtremeValue
    variable = cell_temp
    value_type = max
  []
  [min_temp]
    type = ElementExtremeValue
    variable = cell_temp
    value_type = min
  []
  [avg_temp]
    type = ElementAverageValue
    variable = collated_temp
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
