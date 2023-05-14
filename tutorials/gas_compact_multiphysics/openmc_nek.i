# This input file runs coupled OpenMC Monte Carlo transport, MOOSE heat
# conduction, and NekRS fluid flow and heat transfer.
# This input should be run with:
#
# cardinal-opt -i common_input.i openmc_nek.i

num_layers_for_THM = 150
fluid_blocks = '101 102'
solid_blocks = 'graphite compacts'

unit_cell_power = ${fparse power / (n_bundles * n_coolant_channels_per_block) * unit_cell_height / height}

U_ref = ${fparse mdot / (n_bundles * n_coolant_channels_per_block) / fluid_density / (pi * channel_diameter * channel_diameter / 4.0)}
t0 = ${fparse channel_diameter / U_ref}
nek_dt = 6e-3
N = 1000

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
    execute_on = 'timestep_begin'
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

[ICs]
  [temp]
    type = FunctionIC
    variable = nek_temp
    function = temp_ic
  []
  [solid_temp]
    type = FunctionIC
    variable = solid_temp
    function = temp_ic
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

  temperature_variables = 'solid_temp;   nek_temp'
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
  dt = ${fparse N * nek_dt * t0}

  steady_state_detection = true
  check_aux = true
  steady_state_tolerance = 1e-2
[]

[MultiApps]
  [bison]
    type = TransientMultiApp
    app_type = CardinalApp
    input_files = 'solid_nek.i'
    execute_on = timestep_end
    sub_cycling = true
  []
[]

[Transfers]
  [solid_temp_to_openmc]
    type = MultiAppInterpolationTransfer
    source_variable = T
    variable = solid_temp
    from_multi_app = bison
  []
  [source_to_bison]
    type = MultiAppMeshFunctionTransfer
    source_variable = heat_source
    variable = power
    to_multi_app = bison
    from_postprocessors_to_be_preserved = heat_source
    to_postprocessors_to_be_preserved = power
  []
  [temp_from_nek]
    type = MultiAppMeshFunctionTransfer
    source_variable = nek_bulk_temp
    from_multi_app = bison
    variable = nek_temp
  []
[]

[Postprocessors]
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

[UserObjects]
  [average_power_axial]
    type = LayeredAverage
    variable = heat_source
    direction = z
    num_layers = ${num_layers_for_plots}
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
    hide = 'solid_temp nek_temp'
  []

  [csv]
    type = CSV
    file_base = 'csv_nek/openmc_nek'
  []
[]
