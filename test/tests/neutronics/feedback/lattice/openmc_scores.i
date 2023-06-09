[Mesh]
  type = FileMesh
  file = ../../meshes/pincell.e
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
  power = 500.0
  solid_blocks = '1 3'
  fluid_blocks = '2'
  tally_blocks = '1 2 3'
  tally_type = cell
  solid_cell_level = 1
  fluid_cell_level = 1
  initial_properties = xml
  check_tally_sum = false

  tally_score = heating
  tally_name = heat_source
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
  []
  [fluid_heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
    block = '2'
  []
  [fuel_heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
    block = '1'
  []
  [clad_heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
    block = '3'
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
