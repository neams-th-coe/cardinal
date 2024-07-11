# This input file runs coupled OpenMC Monte Carlo transport, MOOSE heat
# conduction, and NekRS fluid flow and heat transfer.
# This input should be run with:
#
# cardinal-opt -i common_input.i openmc_nek.i

num_layers_for_THM = 150
density_blocks = 'coolant'
temperature_blocks = 'graphite compacts compacts_trimmer_tri'
fuel_blocks = 'compacts compacts_trimmer_tri'

unit_cell_power = ${fparse power / (n_bundles * n_coolant_channels_per_block) * unit_cell_height / height}

U_ref = ${fparse mdot / (n_bundles * n_coolant_channels_per_block) / fluid_density / (pi * channel_diameter * channel_diameter / 4.0)}
t0 = ${fparse channel_diameter / U_ref}
nek_dt = 6e-3
N = 1000

[Mesh]
  [solid]
    type = FileMeshGenerator
    file = solid_mesh_in.e
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
    T = nek_temp
    fp = helium
    execute_on = 'timestep_begin'
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

[Tallies]
  [heat_source]
    type = CellTally
    tally_blocks = ${fuel_blocks}
    tally_name = heat_source

    check_equal_mapped_tally_volumes = true

    tally_trigger = rel_err
    tally_trigger_threshold = 1e-2
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  output = 'unrelaxed_tally_std_dev'

  power = ${unit_cell_power}
  scaling = 100.0
  density_blocks = ${density_blocks}
  cell_level = 1

  relaxation = robbins_monro

  temperature_variables = 'solid_temp;   nek_temp'
  temperature_blocks = '${temperature_blocks}; ${density_blocks}'

  k_trigger = std_dev
  k_trigger_threshold = 7.5e-4
  batches = 40
  max_batches = 100
  batch_interval = 5
[]

[Executioner]
  type = Transient
  dt = ${fparse N * nek_dt * t0}

  # This is somewhat loose, and you should use a tighter tolerance for production runs;
  # we use 1e-2 to obtain a faster tutorial
  steady_state_detection = true
  check_aux = true
  steady_state_tolerance = 1e-2
[]

[MultiApps]
  [bison]
    type = TransientMultiApp
    input_files = 'solid_nek.i'
    execute_on = timestep_end
    sub_cycling = true
  []
[]

[Transfers]
  [solid_temp_to_openmc]
    type = MultiAppGeometricInterpolationTransfer
    source_variable = T
    variable = solid_temp
    from_multi_app = bison
  []
  [source_to_bison]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    source_variable = heat_source
    variable = power
    to_multi_app = bison
    from_postprocessors_to_be_preserved = heat_source
    to_postprocessors_to_be_preserved = power
  []
  [temp_from_nek]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
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
    block = ${fuel_blocks}
  []
  [max_power]
    type = ElementExtremeValue
    variable = heat_source
    value_type = max
    block = ${fuel_blocks}
  []
[]

[UserObjects]
  [average_power_axial]
    type = LayeredAverage
    variable = heat_source
    direction = z
    num_layers = ${num_layers_for_plots}
    block = ${fuel_blocks}
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
