!include common.i

[Mesh]
  [file]
    type = FileMeshGenerator
    file = solid_mesh_in.e
  []
[]

[AuxVariables]
  [cell_density]
    family = MONOMIAL
    order = CONSTANT
  []
  [cell_temperature]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[ICs]
  [temp_fuel]
    type = ConstantIC
    variable = temp
    value = '${T_AVG}'
  []
  [temp_fluid]
    type = FunctionIC
    variable = 'temp'
    function = 'T_fluid'
    block = 'water'
  []
  [density_fluid]
    type = FunctionIC
    variable = 'density'
    function = 'Rho_fluid'
    block = 'water'
  []
[]

[Functions]
  [T_fluid]
    type = ParsedFunction
    expression = '${T_INLET} + ${fparse T_OUTLET - T_INLET} * ((z + ${fparse 0.5 * 1e-2 * HEIGHT}) / ${fparse 1e-2 * HEIGHT})'
  []
  [Rho_fluid]
    type = ParsedFunction
    expression = '${RHO_INLET} + ${fparse RHO_OUTLET - RHO_INLET} * ((z + ${fparse 0.5 * 1e-2 * HEIGHT}) / ${fparse 1e-2 * HEIGHT})'
  []
[]

[AuxKernels]
  [cell_density]
    type = CellDensityAux
    variable = cell_density
  []
  [cell_temperature]
    type = CellTemperatureAux
    variable = cell_temperature
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  particles = 1000
  inactive_batches = 400
  batches = 800

  scaling = 1e2

  power = ${fparse 3000e6 / 273 / (18 * 18)}

  skip_statepoint = true

  cell_level = 1
  temperature_blocks = 'uo2_tri uo2 water'
  density_blocks = 'water'
  mgxs_reference_densities = '1002.0'

  relaxation = 'constant'
  relaxation_factor = 0.5

  [Tallies/heating]
    type = CellTally
    score = 'kappa_fission'
    output = 'unrelaxed_tally_std_dev unrelaxed_tally_rel_error'
    block = 'uo2 uo2_tri'

    normalize_by_global_tally = false
  []
[]

[MultiApps]
  [solid]
    type = TransientMultiApp
    input_files = 'solid.i'
    execute_on = timestep_end
  []
[]

[Transfers]
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
  [temp_to_openmc]
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

[Executioner]
  type = Transient
  num_steps = 5
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
  csv = true
[]
