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

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 1500.0
  temperature_blocks = '0'
  cell_level = 1
  scaling = 100.0

  [Tallies]
    [heat_source]
      type = MeshTally
      mesh_translations = '0 0 0.02
                           0 0 0.06
                           0 0 0.10'
      mesh_template = mesh_in.e
      name = heat_source

      normalize_by_global_tally = false
    []
  []
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
