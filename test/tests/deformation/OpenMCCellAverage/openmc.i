[Mesh]
 [cube]
    type = FileMeshGenerator
    file = '2blckt03.e' # 'openmc_out.e'
 []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  use_displaced_mesh = true
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  tally_type = cell
  cell_level = 0
  temperature_blocks = 'fuel water'
  tally_blocks = 'fuel water'
  # tally_blocks = 'fuel'
  check_tally_sum = false
  normalize_by_global_tally = false
  particles = 50000
  batches = 100
  inactive_batches = 10
  # reuse_source = true
  fixed_mesh = false
  power = 10000.0
  volume_calculation = vol
  skinner = moab
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[AuxVariables]
 # [cell_temperature]
 #   family = MONOMIAL
 #   order = CONSTANT
 # []
  [cell_density]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[ICs]
  [temp]
    type = ConstantIC
    variable = temp
    value = 1000.0
  []
  # [kappa_fission]
  #  type = ConstantIC
  #  variable = kappa_fission
  #  value = 0.01
  # []
[]

# [AuxKernels]
#  [cell_temperature]
#    type = CellTemperatureAux
#    variable = cell_temperature
#  []
# []

[UserObjects]
  [moab]
    type = MoabSkinner
    verbose = true
    temperature = temp
    n_temperature_bins = 15.0
    temperature_min = 500.0
    temperature_max = 1000.0
    # use_displaced_mesh = true
    build_graveyard = true
    output_skins = true
    # material_names = 'mat mat'
    execute_on = 'timestep_end'
    # allow_duplicate_execution_on_initial = 'true'
    graveyard_scale_inner = 1.1
    graveyard_scale_outer = 1.2
  []
  [vol]
    type = OpenMCVolumeCalculation
    n_samples = 5000000
  []
[]

[Postprocessors]
  [heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
    block = 'fuel'
    execute_on = 'transfer initial timestep_end'
  []
  [k]
    type = KEigenvalue
  []
[]

[MultiApps]
  [solid]
    type = TransientMultiApp
    app_type = CardinalApp
    input_files = 'solid.i'
    execute_on = timestep_end
    # execute_on = 'timestep_begin'
  []
[]

[Transfers]
  [heat_source_to_solid]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    to_multi_app = solid
    variable = power
    source_variable = kappa_fission
    from_postprocessors_to_be_preserved = heat_source
    to_postprocessors_to_be_preserved = power
    execute_on = 'timestep_end'
  []
  [temperature_to_openmc]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    from_multi_app = solid
    variable = temp
    source_variable = temp
    # execute_on = 'timestep_begin'
  []
  [dispx]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    source_variable = disp_x
    variable = disp_x
    from_multi_app = solid
    # execute_on = 'timestep_begin'
  []
  [dispy]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    source_variable = disp_y
    variable = disp_y
    from_multi_app = solid
    # execute_on = 'timestep_begin'
  []
  [dispz]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    source_variable = disp_z
    variable = disp_z
    from_multi_app = solid
    # execute_on = 'timestep_begin'
  []
[]

[Executioner]
  type = Transient

  start_time = 0.0
  end_time = 0.075
  dt = 0.0125
[]

[Outputs]
  exodus = true
  overwrite = true
[]
