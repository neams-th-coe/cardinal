# This input file runs coupled OpenMC Monte Carlo transport, MOOSE heat
# conduction, and THM fluid flow and heat transfer.
# This input should be run with:
#
# cardinal-opt -i common_input.i openmc_thm.i

num_layers_for_THM = 150
num_layers = 50
fluid_blocks = '101 102'
solid_blocks = 'graphite compacts'

unit_cell_power = ${fparse power / (n_bundles * n_coolant_channels_per_block) * unit_cell_height / height}

[Mesh]
  [coolant_face]
    type = AnnularMeshGenerator
    nr = 4
    nt = 16
    rmin = 0.0
    rmax = ${fparse channel_diameter / 2.0}
    quad_subdomain_id = 101
    tri_subdomain_id = 102
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = coolant_face
    num_layers = ${num_layers_for_THM}
    direction = '0 0 1'
    heights = '${unit_cell_height}'
    top_boundary = '300' # inlet
    bottom_boundary = '400' # outlet
  []
  [rename] # we need to rename the outer surface of the coolant channel to not conflict with ID 1 which we use in solid_rotated.e
    type = RenameBoundaryGenerator
    input = extrude
    old_boundary = '1'
    new_boundary = '10000'
  []
  [solid]
    type = FileMeshGenerator
    file = solid_rotated.e
  []
  [add]
    type = CombinerGenerator
    inputs = 'solid rename'
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
  [thm_temp_wall]
    family = MONOMIAL
    order = CONSTANT
    block = ${fluid_blocks}
  []
  [flux]
  []
[]

[AuxKernels]
  [cell_temperature]
    type = CellTemperatureAux
    variable = cell_temperature
  []
  [cell_density]
    type = CellDensityAux
    variable = cell_density
  []
  [density]
    type = FluidDensityAux
    variable = density
    p = ${outlet_P}
    T = temp
    fp = helium
    execute_on = 'timestep_begin linear'
  []
[]

[FluidProperties]
  [helium]
    type = IdealGasFluidProperties
    molar_mass = 4e-3
    gamma = 1.668282 # should correspond to  Cp = 5189 J/kg/K
    k = 0.2556
    mu = 3.22639e-5
  []
[]

[ICs]
  [fluid_temp_wall]
    type = FunctionIC
    variable = thm_temp_wall
    function = temp_ic
  []
  [fluid_temp]
    type = FunctionIC
    variable = thm_temp
    function = temp_ic
  []
  [heat_source]
    type = ConstantIC
    variable = heat_source
    block = 'compacts'
    value = ${fparse unit_cell_power / (2.0 * pi * compact_diameter * compact_diameter / 4.0 * unit_cell_height)}
  []
[]

[Functions]
  [temp_ic]
    type = ParsedFunction
    expression = '${inlet_T} + z / ${unit_cell_height} * ${unit_cell_power} / (${mdot} / ${n_bundles} / ${n_coolant_channels_per_block}) / ${fluid_Cp}'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  output = 'unrelaxed_tally_std_dev'
  check_equal_mapped_tally_volumes = true

  power = ${unit_cell_power}
  scaling = 100.0
  solid_blocks = ${solid_blocks}
  fluid_blocks = ${fluid_blocks}
  tally_blocks = 'compacts'
  tally_type = cell
  tally_name = heat_source
  solid_cell_level = 1
  fluid_cell_level = 1

  relaxation = robbins_monro

  temperature_variables = 'solid_temp; thm_temp'
  temperature_blocks = '${solid_blocks}; ${fluid_blocks}'

  k_trigger = std_dev
  k_trigger_threshold = 7.5e-4
  tally_trigger = rel_err
  tally_trigger_threshold = 1e-2
  batches = 40
  max_batches = 100
  batch_interval = 5
[]

[Executioner]
  type = Transient

  steady_state_detection = true
  check_aux = true
  steady_state_tolerance = 1e-2
[]

[MultiApps]
  [bison]
    type = TransientMultiApp
    app_type = CardinalApp
    input_files = 'solid_thm.i'
    execute_on = timestep_begin
  []
  [thm]
    type = FullSolveMultiApp
    app_type = CardinalApp
    input_files = 'thm.i'
    execute_on = timestep_end
    max_procs_per_app = 1
    bounding_box_padding = '0.1 0.1 0'
  []
[]

[Transfers]
  [solid_temp_to_openmc]
    type = MultiAppGeometricInterpolationTransfer
    source_variable = T
    variable = solid_temp
    from_multi_app = bison
  []
  [heat_flux_to_openmc]
    type = MultiAppNearestNodeTransfer
    fixed_meshes = true
    source_variable = flux
    variable = flux
    from_multi_app = bison
    source_boundary = 'fluid_solid_interface'
    target_boundary = 'fluid_solid_interface'
    from_postprocessors_to_be_preserved = flux_integral
    to_postprocessors_to_be_preserved = flux_integral
  []
  [source_to_bison]
    type = MultiAppShapeEvaluationTransfer
    source_variable = heat_source
    variable = power
    to_multi_app = bison
    from_postprocessors_to_be_preserved = heat_source
    to_postprocessors_to_be_preserved = power
  []
  [thm_temp_to_bison]
    type = MultiAppGeometricInterpolationTransfer
    source_variable = thm_temp_wall
    variable = fluid_temp
    to_multi_app = bison
  []

  [q_wall_to_thm]
    type = MultiAppGeneralFieldUserObjectTransfer
    variable = q_wall
    to_multi_app = thm
    source_user_object = q_wall_avg
  []
  [T_wall_from_thm]
    type = MultiAppNearestNodeTransfer
    source_variable = T_wall
    from_multi_app = thm
    variable = thm_temp_wall
    fixed_meshes = true
  []
  [T_bulk_from_thm]
    type = MultiAppNearestNodeTransfer
    source_variable = T
    from_multi_app = thm
    variable = thm_temp
    fixed_meshes = true
  []
[]

[Postprocessors]
  [flux_integral]
    type = SideIntegralVariablePostprocessor
    variable = flux
    boundary = 'fluid_solid_interface'
    execute_on = 'transfer'
  []
  [heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
    execute_on = 'transfer initial timestep_end'
  []
  [max_tally_err]
    type = TallyRelativeError
    value_type = max
  []
  [k]
    type = KEigenvalue
  []
  [k_std_dev]
    type = KStandardDeviation
  []
  [min_power]
    type = ElementExtremeValue
    variable = heat_source
    value_type = min
    block = 'compacts'
  []
  [max_power]
    type = ElementExtremeValue
    variable = heat_source
    value_type = max
    block = 'compacts'
  []
[]


[AuxVariables]
  [q_wall]
  []
[]

[AuxKernels]
  [q_wall]
    type = SpatialUserObjectAux
    variable = q_wall
    user_object = q_wall_avg
  []
[]

[UserObjects]
  [q_wall_avg]
    type = LayeredSideAverage
    boundary = fluid_solid_interface
    variable = flux

    # Note: make this to match the num_elems in the channel
    direction = z
    num_layers = ${num_layers_for_THM}

    direction_min = 0.0
    direction_max = ${unit_cell_height}
  []
  [average_power_axial]
    type = LayeredAverage
    variable = heat_source
    direction = z
    num_layers = ${num_layers}
    block = 'compacts'
  []
[]

[VectorPostprocessors]
  [power_avg]
    type = SpatialUserObjectVectorPostprocessor
    userobject = average_power_axial
  []
[]

[Outputs]
  [out]
    type = Exodus
    hide = 'solid_temp thm_temp'
  []

  [csv]
    type = CSV
    file_base = 'csv_thm/openmc_thm'
  []
[]
