[Mesh]
  type = FileMesh
  file = ../../meshes/pincell.e
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
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 500.0
  temperature_blocks = '1 2 3'
  density_blocks = '2'
  verbose = true
  cell_level = 0
[]

[Executioner]
  type = Transient
[]

[Postprocessors]
  [kappa_fission]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
  []
  [fluid_kappa_fission]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
    block = '2'
  []
  [solid_kappa_fission]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
    block = '1 3'
  []
[]

[Outputs]
  exodus = true
[]
