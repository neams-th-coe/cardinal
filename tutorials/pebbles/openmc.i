[Mesh]
  [pebble]
    type = SphereMeshGenerator
    nr = 2
    radius = 0.015
  []
  [repeat]
    type = CombinerGenerator
    inputs = pebble
    positions = '0 0 0.02
                 0 0 0.06
                 0 0 0.10'
  []
[]

[AuxVariables]
  [cell_temperature]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_temperature]
    type = CellTemperatureAux
    variable = cell_temperature
  []
[]

[Tallies]
  [heat_source]
    type = CellTally
    tally_blocks = '0'
    tally_name = heat_source
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 1500.0
  temperature_blocks = '0'
  cell_level = 1
  scaling = 100.0
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
  csv = true
[]

[Postprocessors]
  [heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
  []
  [max_tally_rel_err]
    type = TallyRelativeError
  []
  [k]
    type = KEigenvalue
  []
[]
