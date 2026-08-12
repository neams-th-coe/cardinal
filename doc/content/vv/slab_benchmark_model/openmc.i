P = ${units 1.0e22 eV/s}
eV_to_J = ${units 1.602176634e-19 J/eV}
initial_temperature = ${units 293 K}

[Mesh]
  [file]
    type = FileMeshGenerator
    file = slab20v.e
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  use_displaced_mesh = true
[]

[Variables]
  [disp_x]
  []
  [disp_y]
    initial_condition = 0.0
  []
  [disp_z]
    initial_condition = 0.0
  []
[]

[AuxVariables]
  [bins]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [bins]
    type = SkinnedBins
    variable = bins
    skinner = moab
  []
[]

[ICs]
  [temperature]
    type = FunctionIC
    variable = temperature
    function = ${initial_temperature}
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  source_rate_normalization = 'kappa_fission'
  inactive_batches = 150
  batches = 300
  particles = 10000000
  power = '${fparse P*eV_to_J}'
  cell_level = 0
  temperature_variables = 'temperature'
  temperature_blocks = '1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20'
  volume_calculation = vol
  skinner = moab
  [Tallies]
    [kappa_fission]
      type = CellTally
      estimator = collision
      block = '1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20'
      name = kappa_fission
      output = unrelaxed_tally_std_dev
    []
    [flux]
      type = CellTally
      estimator = collision
      block = '1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20'
      score = flux
      output = unrelaxed_tally_std_dev
    []
  []
[]

[Executioner]
  type = Transient
  start_time = 0.0
  end_time = 1e5
  dt = 50
[]

[MultiApps]
  [solid]
    type = TransientMultiApp
    input_files = 'solid.i'
    execute_on = timestep_end
  []
[]

[Transfers]
  [kappa_fission_to_solid]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    to_multi_app = solid
    variable = kappa_fission
    source_variable = kappa_fission
    from_postprocessors_to_be_preserved = heat_source
    to_postprocessors_to_be_preserved = source_integral
    execute_on = timestep_end
    displaced_source_mesh = true
    displaced_target_mesh = true
  []
  [temperature_from_solid]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = solid
    variable = temperature
    source_variable = temperature_praux
    execute_on = timestep_end
    displaced_source_mesh = true
    displaced_target_mesh = true
  []
  [dispx]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = disp_praux
    variable = disp_x
    from_multi_app = solid
    displaced_source_mesh = true
    displaced_target_mesh = true
  []
[]

[Outputs]
  exodus = true
  csv = true
[]

[Postprocessors]
  [heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
    execute_on = 'transfer initial timestep_end'
  []
  [k]
    type = KEigenvalue
  []
  [k_std_dev]
    type = KEigenvalue
    output = 'std_dev'
  []
[]

[VectorPostprocessors]
  [temperature]
    type = ElementValueSampler
    variable = 'temperature'
    sort_by = x
    execute_on = 'FINAL'
  []
[]

[UserObjects]
  [vol]
    type = OpenMCVolumeCalculation
    n_samples = 200000
  []
  [moab]
    type = MoabSkinner
    execute_on = 'timestep_begin'
    verbose = true
    temperature = temperature
    n_temperature_bins = 20.0
    temperature_min = 0
    temperature_max = 1000
    build_graveyard = true
    graveyard_scale_inner = 1.5
    graveyard_scale_outer = 1.6
  []
[]
