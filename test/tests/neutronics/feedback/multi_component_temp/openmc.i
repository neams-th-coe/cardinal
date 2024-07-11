[Mesh]
  type = FileMesh
  file = ../../meshes/pincell.e
[]

[AuxVariables]
  [cell_temp]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_temp]
    type = CellTemperatureAux
    variable = cell_temp
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
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '1'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 500.0
  cell_level = 1
  verbose = true

  temperature_variables = 'solid_temp; fluid_temp'
  temperature_blocks = '1 3; 2'
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [max_temp_1]
    type = ElementExtremeValue
    variable = cell_temp
    value_type = max
    block = '1'
  []
  [min_temp_1]
    type = ElementExtremeValue
    variable = cell_temp
    value_type = min
    block = '1'
  []
  [max_temp_2]
    type = ElementExtremeValue
    variable = cell_temp
    value_type = max
    block = '2'
  []
  [min_temp_2]
    type = ElementExtremeValue
    variable = cell_temp
    value_type = min
    block = '2'
  []
  [max_temp_3]
    type = ElementExtremeValue
    variable = cell_temp
    value_type = max
    block = '3'
  []
  [min_temp_3]
    type = ElementExtremeValue
    variable = cell_temp
    value_type = min
    block = '3'
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
