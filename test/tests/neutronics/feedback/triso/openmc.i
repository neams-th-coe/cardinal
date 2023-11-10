[Mesh]
  [solid]
    type = FileMeshGenerator
    file = solid.e
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
[]

[ICs]
  [temp]
    type = FunctionIC
    variable = temp
    function = temp_ic
  []
[]

[Functions]
  [temp_ic]
    type = ParsedFunction
    expression = '500.0'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  check_equal_mapped_tally_volumes = true
  identical_cell_fills = '2'
  check_identical_cell_fills = true

  power = 100.0
  scaling = 100.0
  temperature_blocks = '1 2'
  tally_blocks = '2'
  tally_type = cell
  cell_level = 1
[]


[Postprocessors]
  [k]
    type = KEigenvalue
  []
  [max_tally_rel_err]
    type = TallyRelativeError
    value_type = max
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true

  # get a smaller gold file
  hide = 'temp'
[]
