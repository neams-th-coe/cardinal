inlet_T  = 573.0           # inlet temperature
power = 1e3                # total power (W)
Re = 500.0                 # Reynolds number
outlet_P = 1e6

height = 0.5               # total height of the domain
Df = 0.825e-2              # fuel diameter
pin_diameter = 0.97e-2     # pin outer diameter
pin_pitch = 1.28e-2        # pin pitch

mu = 8.8e-5                # fluid dynamic viscosity
rho = 723.6                # fluid density
Cp = 5512.0                # fluid isobaric specific heat capacity

Rf = ${fparse Df / 2.0}

flow_area = ${fparse pin_pitch * pin_pitch - pi * pin_diameter * pin_diameter / 4.0}
wetted_perimeter = ${fparse pi * pin_diameter}
hydraulic_diameter = ${fparse 4.0 * flow_area / wetted_perimeter}

U_ref = ${fparse Re * mu / rho / hydraulic_diameter}
mdot = ${fparse rho * U_ref * flow_area}
dT = ${fparse power / mdot / Cp}

[Mesh]
  [solid]
    type = FileMeshGenerator
    file = solid_in.e
  []
[]

[AuxVariables]
# These auxiliary variables are all just for visualizing the solution and
# the mapping - none of these are part of the calculation sequence

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
[]

[AuxKernels]
  [material_id]
    type = CellMaterialIDAux
    variable = material_id
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
    T = nek_temp
    fp = sodium
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[FluidProperties]
  [sodium]
    type = SodiumSaturationFluidProperties
  []
[]

[ICs]
  [nek_temp]
    type = FunctionIC
    variable = nek_temp
    function = temp_ic
  []
  [solid_temp]
    type = FunctionIC
    variable = solid_temp
    function = temp_ic
  []
  [heat_source]
    type = ConstantIC
    variable = heat_source
    block = '2'
    value = ${fparse power / (pi * Rf * Rf * height)}
  []
[]

[Functions]
  [temp_ic]
    type = ParsedFunction
    expression = '${inlet_T} + z / ${height} * ${dT}'
  []
[]

[Tallies]
  [heat_source]
    type = CellTally
    tally_blocks = '2'
    tally_name = heat_source

    check_equal_mapped_tally_volumes = true

    tally_trigger = rel_err
    tally_trigger_threshold = 2e-2
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  output = 'unrelaxed_tally_std_dev'

  power = ${power}
  scaling = 100.0
  density_blocks = '1'
  cell_level = 0

  # This automatically creates these variables and will read from the non-default choice of 'temp'
  temperature_variables = 'solid_temp; nek_temp'
  temperature_blocks    = '2 3;        1'

  relaxation = robbins_monro

  # Set some parameters for when we terminate the OpenMC solve in each iteration;
  # this will run a minimum of 30 batches, and after that, terminate once reaching
  # the specified std. dev. of k and rel. err. of the fission tally
  inactive_batches = 20
  batches = 30
  k_trigger = std_dev
  k_trigger_threshold = 7.5e-4
  batch_interval = 50
  max_batches = 1000
[]

[MultiApps]
  [bison]
    type = TransientMultiApp
    input_files = 'bison.i'
    execute_on = timestep_begin
    sub_cycling = true
  []
[]

[Transfers]
  [solid_temp_to_openmc]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    source_variable = T
    variable = solid_temp
    from_multi_app = bison
  []
  [source_to_bison]
    type = MultiAppCopyTransfer
    source_variable = heat_source
    variable = power
    to_multi_app = bison
  []
  [temp_from_nek]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    source_variable = nek_temp
    from_multi_app = bison
    variable = nek_temp
  []
[]

[Outputs]
  exodus = true
  csv = true
[]

[Executioner]
  type = Transient
  dt = 0.5

  steady_state_detection = true
  check_aux = true
  steady_state_tolerance = 1e-3
[]

[Postprocessors]
  [max_tally_rel_err]
    type = TallyRelativeError
    value_type = max
  []
  [k]
    type = KEigenvalue
  []
  [k_std_dev]
    type = KStandardDeviation
  []
[]
