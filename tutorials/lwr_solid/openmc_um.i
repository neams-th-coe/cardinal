[Mesh]
  [file]
    type = FileMeshGenerator
    file = mesh_in.e
  []

  allow_renumbering = false
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
    type = MeshTally
    tally_name = heat_source
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = ${fparse 3000e6 / 273 / (17 * 17)}
  temperature_blocks = '1 2 3'
  normalize_by_global_tally = false
  cell_level = 0

  particles = 20000
  inactive_batches = 500
  batches = 10000
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
  [max_heat_source]
    type = ElementExtremeValue
    variable = heat_source
  []
[]
