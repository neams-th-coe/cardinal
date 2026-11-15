!include common.i

[Mesh]
  [file]
    type = FileMeshGenerator
    file = openmc_mesh_in.e
  []
[]

[AuxVariables]
  [cell_temperature]
    family = MONOMIAL
    order = CONSTANT
  []
  [cell_density]
    family = MONOMIAL
    order = CONSTANT
  []
  [average_power]
    family = MONOMIAL
    order = CONSTANT
  []
  [linear_power]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[ICs]
  [temp_fuel]
    type = ConstantIC
    variable = temp
    value = '${T_INLET}'
    block = 'uo2_tri uo2'
  []
  [temp_water]
    type = ConstantIC
    variable = temp
    value = '${T_INLET}'
    block = 'water'
  []
  [density_water]
    type = ConstantIC
    variable = density
    value = '${RHO_INIT}'
    block = 'water'
  []
  [linear_power]
    type = ConstantIC
    variable = linear_power
    value = '${fparse P_PIN / (0.01 * HEIGHT)}'
    block = 'uo2_tri uo2'
  []
[]

[AuxKernels]
  [cell_temperature]
    type = CellTemperatureAux
    variable = cell_temperature
    execute_on = 'TIMESTEP_END'
  []
  [cell_density]
    type = CellDensityAux
    variable = cell_density
    execute_on = 'TIMESTEP_END'
  []
  [average_power]
    type = SpatialUserObjectAux
    variable = average_power
    user_object = average_power
    execute_on = 'TIMESTEP_END'
    block = 'uo2_tri uo2'
  []
  [linear_power]
    type = ParsedAux
    variable = linear_power
    # Multiply the volumetric average power by the cross-sectional area of the fuel pin to obtain
    # a linear power.
    expression = 'average_power * ${fparse pi * (0.01 * (R_FUEL + T_F_C_GAP + T_ZR_CLAD))^2}'
    coupled_variables = 'average_power'
    block = 'uo2_tri uo2'
    execute_on = 'TIMESTEP_END'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  particles = 100
  inactive_batches = 400
  batches = 800

  scaling = 100.0

  power = ${P_PIN}

  cell_level = 1
  temperature_blocks = 'uo2_tri uo2 water'
  density_blocks = 'water'
  mgxs_reference_densities_by_block = '${RHO_INIT}'

  relaxation = 'constant'
  relaxation_factor = 0.5

  [Tallies/heating]
    type = CellTally
    score = 'kappa_fission'
    output = 'unrelaxed_tally_std_dev unrelaxed_tally_rel_error'
    block = 'uo2 uo2_tri'
  []
[]

[MultiApps]
  [solid]
    type = TransientMultiApp
    input_files = 'solid.i'
    execute_on = timestep_end
  []
  [subchannel]
    type = FullSolveMultiApp
    input_files = 'subchannel.i'
    max_procs_per_app = 1
    execute_on = 'TIMESTEP_BEGIN'
  []
[]

[Transfers]
  [linear_heat_rate_to_subchannel]
    type = MultiAppGeneralFieldNearestLocationTransfer
    variable = q_prime
    source_variable = linear_power
    to_multi_app = subchannel
    greedy_search = true
    use_bounding_boxes = false
    to_blocks = 'fuel_pins'
    from_blocks = 'uo2_tri uo2'
  []
  [fluid_temperature_from_subchannel]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = T
    variable = temp
    from_multi_app = subchannel
    greedy_search = true
    use_bounding_boxes = false
    to_blocks = 'water'
    from_blocks = 'subchannel'
  []
  [fluid_density_from_subchannel]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = rho
    variable = density
    from_multi_app = subchannel
    greedy_search = true
    use_bounding_boxes = false
    to_blocks = 'water'
    from_blocks = 'subchannel'
  []
  [clad_surface_temperature_to_conduction]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = subchannel
    to_multi_app = solid
    source_variable = Tpin
    variable = T_wall
  []

  [heat_source_to_solid]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    to_multi_app = solid
    variable = heat_source
    source_variable = kappa_fission
    from_postprocessors_to_be_preserved = source_integral
    to_postprocessors_to_be_preserved = source_integral
    from_blocks = 'uo2_tri uo2'
    to_blocks = 'uo2_tri uo2'
  []
  [solid_temp_to_openmc]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    from_multi_app = solid
    variable = temp
    source_variable = temp
    from_blocks = 'uo2_tri uo2'
    to_blocks = 'uo2_tri uo2'
  []
[]

[Postprocessors]
  [max_rel_err]
    type = TallyRelativeError
    value_type = max
    tally_score = kappa_fission
  []
  [min_rel_err]
    type = TallyRelativeError
    value_type = min
    tally_score = kappa_fission
  []
  [avg_rel_err]
    type = TallyRelativeError
    value_type = average
    tally_score = kappa_fission
  []

  [k]
    type = KEigenvalue
    output = 'mean'
    value_type = 'tracklength'
  []
  [k_std_dev]
    type = KEigenvalue
    output = 'std_dev'
    value_type = 'tracklength'
  []

  [source_integral]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
    execute_on = 'TRANSFER TIMESTEP_END'
    block = 'uo2_tri uo2'
  []
[]

[UserObjects]
  [average_power]
    type = LayeredAverage
    direction = z
    num_layers = ${AXIAL_LAYERS}
    variable = kappa_fission
    execute_on = 'TIMESTEP_END'
    block = 'uo2_tri uo2'
  []
  [average_power_rel]
    type = LayeredAverage
    direction = z
    num_layers = ${AXIAL_LAYERS}
    variable = kappa_fission_rel_error
    execute_on = 'TIMESTEP_END'
    block = 'uo2_tri uo2'
  []
  [average_fuel_temp]
    type = LayeredAverage
    direction = z
    num_layers = ${AXIAL_LAYERS}
    variable = temp
    execute_on = 'TIMESTEP_END'
    block = 'uo2_tri uo2'
  []
  [average_water_temp]
    type = LayeredAverage
    direction = z
    num_layers = ${AXIAL_LAYERS}
    variable = temp
    execute_on = 'TIMESTEP_END'
    block = 'water'
  []
  [average_water_density]
    type = LayeredAverage
    direction = z
    num_layers = ${AXIAL_LAYERS}
    variable = density
    execute_on = 'TIMESTEP_END'
    block = 'water'
  []
[]

[VectorPostprocessors]
  [kappa_fission_z_avg]
    type = SpatialUserObjectVectorPostprocessor
    userobject = average_power
  []
  [kappa_fission_rel_z_avg]
    type = SpatialUserObjectVectorPostprocessor
    userobject = average_power_rel
  []
  [fuel_temp_z_avg]
    type = SpatialUserObjectVectorPostprocessor
    userobject = average_fuel_temp
  []
  [coolant_temp_z_avg]
    type = SpatialUserObjectVectorPostprocessor
    userobject = average_water_temp
  []
  [coolant_density_z_avg]
    type = SpatialUserObjectVectorPostprocessor
    userobject = average_water_density
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
  csv = true
[]
