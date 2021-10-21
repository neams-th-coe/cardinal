# This input file runs coupled OpenMC Monte Carlo transport, MOOSE heat
# conduction, and THM fluid flow and heat transfer.
# This input should be run with:
#
# cardinal-opt -i common_input.i openmc.i

num_layers_for_THM = 150

[Mesh]
  # mesh mirror for the solid regions
  [solid]
    type = FileMeshGenerator
    file = solid_mesh_in.e
  []

  # create a mesh for a single coolant channel; because we will receive uniform
  # temperatures and densities from THM on each x-y plane, we can use a very coarse
  # mesh in the radial direction
  [coolant_face]
    type = AnnularMeshGenerator
    nr = 1
    nt = 16
    rmin = 0.0
    rmax = ${fparse channel_diameter / 2.0}
  []
  [extrude]
    type = FancyExtruderGenerator
    input = coolant_face
    num_layers = ${num_layers_for_THM}
    direction = '0 0 1'
    heights = '${height}'
    top_boundary = '300' # inlet
    bottom_boundary = '400' # outlet
  []
  [rename]
    type = RenameBlockGenerator
    input = extrude
    old_block_id = '1'
    new_block_id = '101'
  []

  # repeat the coolant channels and then combine together to get a combined mesh mirror
  [repeat]
    type = CombinerGenerator
    inputs = rename
    positions_file = coolant_channel_positions.txt
  []
  [add]
    type = CombinerGenerator
    inputs = 'solid repeat'
  []
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
  [material_id]
    family = MONOMIAL
    order = CONSTANT
  []
  [cell_temperature]
    family = MONOMIAL
    order = CONSTANT
  []
  [cell_density]
    family = MONOMIAL
    order = CONSTANT
  []
  [solid_temp]
    block = 'graphite compacts poison'
  []
  [thm_temp]
    block = '101'
  []
  [thm_temp_wall]
    block = '101'
  []
  [flux]
  []

  # just for postprocessing purposes
  [thm_pressure]
    block = '101'
  []
  [thm_velocity]
    block = '101'
  []
  [z]
    family = MONOMIAL
    order = CONSTANT
    block = 'compacts'
  []
[]

[AuxKernels]
  [material_id]
    type = CellMaterialIDAux
    variable = material_id
  []
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
  [temp_in_solids]
    type = ParsedAux
    variable = temp
    block = 'graphite compacts poison'
    function = 'solid_temp'
    args = 'solid_temp'
    execute_on = 'timestep_begin linear'
  []
  [temp_in_fluid]
    type = ParsedAux
    variable = temp
    block = '101'
    function = 'thm_temp'
    args = 'thm_temp'
    execute_on = 'timestep_begin linear'
  []
  [z]
    type = ParsedAux
    variable = z
    use_xyzt = true
    function = 'z'
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
    value = ${fparse power / (n_bundles * n_fuel_compacts_per_block) / (pi * compact_diameter * compact_diameter / 4.0 * height)}
  []
[]

[Functions]
  [temp_ic]
    type = ParsedFunction
    value = '${inlet_T} + (${height} - z) / ${height} * ${power} / ${mdot} / ${fluid_Cp}'
  []
[]

[Modules]
  [FluidProperties]
    [helium]
      type = IdealGasFluidProperties
      molar_mass = 4e-3
      gamma = 1.668282 # should correspond to  Cp = 5189 J/kg/K
      k = 0.2556
      mu = 3.22639e-5
    []
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  output = 'fission_tally_std_dev'
  check_equal_mapped_tally_volumes = true

  identical_tally_cell_fills = true
  check_identical_tally_cell_fills = false

  power = ${fparse power / n_bundles}
  scaling = 100.0
  solid_blocks = '1 2 4'
  fluid_blocks = '101'
  tally_blocks = '2'
  tally_type = cell
  solid_cell_level = 1
  fluid_cell_level = 1

  relaxation = constant
  relaxation_factor = 0.5
[]

[Postprocessors]
  [flux_integral]
    type = SideIntegralVariablePostprocessor
    variable = flux
    boundary = 'fluid_solid_interface'
    execute_on = 'transfer linear'
  []
  [heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
    execute_on = 'transfer initial timestep_end'
  []
  [max_tally_rel_err]
    type = FissionTallyRelativeError
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
  [z_max_power]
    type = ElementExtremeValue
    proxy_variable = heat_source
    variable = z
    block = 'compacts'
  []
  [max_Tf]
    type = ElementExtremeValue
    variable = thm_temp
    block = '101'
  []
  [P_in]
    type = SideAverageValue
    variable = thm_pressure
    boundary = '300'
  []
  [pressure_drop]
    type = LinearCombinationPostprocessor
    pp_names = 'P_in'
    pp_coefs = '1.0'
    b = '${fparse -outlet_P}'
  []
[]

[MultiApps]
  [bison]
    type = TransientMultiApp
    app_type = CardinalApp
    input_files = 'solid.i'
    execute_on = timestep_begin
    sub_cycling = true
  []
  [thm]
    type = FullSolveMultiApp
    app_type = CardinalApp
    input_files = 'thm.i'
    execute_on = timestep_end
    max_procs_per_app = 1
    bounding_box_padding = '0.1 0.1 0'
    positions_file = coolant_channel_positions.txt
    output_in_position = true
  []
[]

[Transfers]
  [solid_temp_to_openmc]
    type = MultiAppInterpolationTransfer
    source_variable = T
    variable = solid_temp
    direction = from_multiapp
    multi_app = bison
  []
  [heat_flux_to_openmc]
    type = MultiAppNearestNodeTransfer
    fixed_meshes = true
    source_variable = flux
    variable = flux
    direction = from_multiapp
    multi_app = bison
    source_boundary = 'fluid_solid_interface'
    target_boundary = 'fluid_solid_interface'
    from_postprocessors_to_be_preserved = flux_integral
    to_postprocessors_to_be_preserved = flux_integral
  []
  [source_to_bison]
    type = MultiAppMeshFunctionTransfer
    source_variable = heat_source
    variable = power
    direction = to_multiapp
    multi_app = bison
    from_postprocessors_to_be_preserved = heat_source
    to_postprocessors_to_be_preserved = power
  []
  [thm_temp_to_bison]
    type = MultiAppInterpolationTransfer
    source_variable = thm_temp_wall
    variable = thm_temp
    direction = to_multiapp
    multi_app = bison
  []

  [q_wall_to_thm]
    type = MultiAppUserObjectTransfer
    variable = q_wall
    direction = to_multiapp
    multi_app = thm
    user_object = q_wall_avg
  []
  [T_wall_from_thm]
    type = MultiAppNearestNodeTransfer
    source_variable = T_wall
    direction = from_multiapp
    multi_app = thm
    variable = thm_temp_wall
    fixed_meshes = true
    target_boundary = 'fluid_solid_interface'
  []
  [T_bulk_from_thm]
    type = MultiAppNearestNodeTransfer
    source_variable = T
    direction = from_multiapp
    multi_app = thm
    variable = thm_temp
    fixed_meshes = true
  []

  # just for postprocessing purposes
  [pressure_from_thm]
    type = MultiAppNearestNodeTransfer
    source_variable = p
    direction = from_multiapp
    multi_app = thm
    variable = thm_pressure
    fixed_meshes = true
  []
  [velocity_from_thm]
    type = MultiAppNearestNodeTransfer
    source_variable = vel_z
    direction = from_multiapp
    multi_app = thm
    variable = thm_velocity
    fixed_meshes = true
  []
[]

[UserObjects]
  [q_wall_avg]
    type = NearestPointLayeredSideAverage
    boundary = fluid_solid_interface
    variable = flux

    # Note: make this to match the num_elems in the channel
    direction = z
    num_layers = ${num_layers_for_THM}
    points_file = coolant_channel_positions.txt

    direction_min = 0.0
    direction_max = ${height}
  []
  [average_power_axial]
    type = NearestPointLayeredAverage
    variable = heat_source
    direction = z
    num_layers = ${num_layers_for_plots}
    points = '0.0 0.0 0.0'
    block = 'compacts'
  []
  [average_fluid_axial]
    type = NearestPointLayeredAverage
    variable = thm_temp
    direction = z
    num_layers = ${num_layers_for_plots}
    points = '0.0 0.0 0.0'
    block = '101'
  []
  [average_wall_axial]
    type = NearestPointLayeredAverage
    variable = thm_temp_wall
    direction = z
    num_layers = ${num_layers_for_plots}
    points = '0.0 0.0 0.0'
    block = '101'
  []
  [average_pressure]
    type = NearestPointLayeredAverage
    variable = thm_pressure
    direction = z
    num_layers = ${num_layers_for_plots}
    points = '0.0 0.0 0.0'
    block = '101'
  []
  [average_axial_velocity]
    type = NearestPointLayeredAverage
    variable = thm_velocity
    direction = z
    num_layers = ${num_layers_for_plots}
    points = '0.0 0.0 0.0'
    block = '101'
  []
[]

[VectorPostprocessors]
  [power_avg]
    type = SpatialUserObjectVectorPostprocessor
    userobject = average_power_axial
  []
  [fluid_avg]
    type = SpatialUserObjectVectorPostprocessor
    userobject = average_fluid_axial
  []
  [fluid_wall_avg]
    type = SpatialUserObjectVectorPostprocessor
    userobject = average_wall_axial
  []
  [pressure_avg]
    type = SpatialUserObjectVectorPostprocessor
    userobject = average_pressure
  []
  [velocity_avg]
    type = SpatialUserObjectVectorPostprocessor
    userobject = average_axial_velocity
  []
[]

[Executioner]
  type = Transient
  num_steps = 10
[]

[Outputs]
  exodus = true
  csv = true
  hide = 'P_in flux_integral z'
[]
