[Mesh]
  [pebble]
    type = FileMeshGenerator
    file = sphere_in_m.e
  []
  [repeat]
    type = CombinerGenerator
    inputs = pebble
    positions = '0 0 0.10
                 0 0 0.02
                 0 0 0.06'
  []
  [set_block_ids]
    type = SubdomainIDGenerator
    input = repeat
    subdomain_id = 0
  []
[]


[AuxVariables]
  [cell_id]
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
  [cell_temperature]
    type = CellTemperatureAux
    variable = cell_temperature
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true

  power = 1500.0

  solid_blocks = '0'
  tally_blocks = '0'
  tally_type = cell

  solid_cell_level = 2

  scaling = 100.0
[]

[Executioner]
  type = Transient

  # we need this to match the quadrature used in the receiving MOOSE app
  # (does not exist in this input file) so that the elem->volume() computed
  # for normalization within OpenMCCellAverageProblem is the same as in the
  # receiving MOOSE app.
  [Quadrature]
    type = GAUSS
    order = THIRD
  []
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  [heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
  []
  [max_T]
    type = ElementExtremeValue
    variable = cell_temperature
  []
  [min_T]
    type = ElementExtremeValue
    variable = cell_temperature
    value_type = min
  []
[]
