[Mesh]
  type = FileMesh
  file = ../../meshes/pincell.e
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '1 2 3'
    tally_score = heating
    tally_name = heat_source
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 500.0

  temperature_blocks = '1 2 3'
  density_blocks = '2'

  cell_level = 1

  check_tally_sum = false

  initial_properties = xml
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
