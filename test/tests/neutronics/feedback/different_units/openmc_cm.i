[Mesh]
  type = FileMesh
  file = sphere_in_cm.e
[]

[AuxVariables]
  [cell_temperature]
    family = MONOMIAL
    order = CONSTANT
  []
  [material_id]
    family = MONOMIAL
    order = CONSTANT
  []
  [cell_density]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [material_id]
    type = CellMaterialIDAux
    variable = material_id
  []
  [cell_temperature]
    type = CellTemperatureAux
    variable = cell_temperature
    execute_on = 'timestep_end'
  []
  [cell_density]
    type = CellDensityAux
    variable = cell_density
    execute_on = 'timestep_end'
  []
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '1'
    tally_name = 'heat_source'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 1e6

  temperature_blocks = '1'
  density_blocks = '1'

  cell_level = 0
[]

[Executioner]
  type = Transient
[]

[Postprocessors]
  [heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
  []
[]

[Outputs]
  exodus = true
[]
