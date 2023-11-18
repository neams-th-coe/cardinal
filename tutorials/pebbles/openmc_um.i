[Mesh]
  [pebble]
    type = FileMeshGenerator
    file = mesh_in.e
  []
  [repeat]
    type = CombinerGenerator
    inputs = pebble
    positions = '0 0 0.02
                 0 0 0.06
                 0 0 0.10'
  []

  allow_renumbering = false
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

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 1500.0
  temperature_blocks = '0'
  tally_type = mesh
  tally_name = heat_source
  mesh_template = mesh_in.e
  mesh_translations = '0 0 0.02
                       0 0 0.06
                       0 0 0.10'
  normalize_by_global_tally = false
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
