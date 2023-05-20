[Mesh]
  [file]
    type = FileMeshGenerator
    file = mesh_in.e
  []

  allow_renumbering = false
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
  tally_type = mesh
  mesh_template = mesh_in.e
  tally_name = 'heat_source'
  solid_cell_level = 0
  solid_blocks = '2 3'
  check_tally_sum = false
  normalize_by_global_tally = false
  check_zero_tallies = false

  power = 1000.0
  volume_calculation = vol
[]

[UserObjects]
  [vol]
    type = OpenMCVolumeCalculation
    n_samples = 200000
  []
[]

[Postprocessors]
  [heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
  []
  [k]
    type = KEigenvalue
  []
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
  csv = true
[]
