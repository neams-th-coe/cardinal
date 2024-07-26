[Mesh]
  [file]
    type = FileMeshGenerator
    file = tokamak.e
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

[Problem]
  type = OpenMCCellAverageProblem
  output = unrelaxed_tally_std_dev
  scaling = 100.0

  tally_type = mesh
  mesh_template = tokamak.e
  tally_score = 'heating_local H3_production'
  source_strength = 2e18

  cell_level = 0
  temperature_blocks = 'plasma_facing multiplier breeder divertor vacuum_vessel'

  # this is a low number of particles; you will want to increase in order to obtain
  # high-quality results
  first_iteration_particles = 1000
  relaxation = dufek_gudowski

  skinner = moab
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    temperature_min = 0
    temperature_max = 2000
    n_temperature_bins = 40
    temperature = temp
    build_graveyard = true
    output_skins = true
  []
[]

[Postprocessors]
  [heating]
    type = ElementIntegralVariablePostprocessor
    variable = heating_local
  []
  [tritium_production]
    type = ElementIntegralVariablePostprocessor
    variable = H3_production
  []
  [tritium_error]
    type = TallyRelativeError
    tally_score = H3_production
    value_type = average
  []
  [heating_error]
    type = TallyRelativeError
    tally_score = heating_local
    value_type = average
  []
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
  csv = true
[]
