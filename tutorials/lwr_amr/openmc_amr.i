# The number of refinement cycles.
num_cycles = 5
# The upper error fraction.
r_error_fraction = 0.3
# The upper limit of statistical relative error.
r_stat_error = 1e-2
# The lower limit of statistical relative error.
c_stat_error = 1e-1

[Mesh]
  [file]
    type = FileMeshGenerator
    file = mesh_in.e
  []
[]

[Adaptivity]
  marker = error_combo
  steps = ${num_cycles}

  [Indicators/optical_depth]
    type = ElementOpticalDepthIndicator
    rxn_rate = 'fission'
    h_type = 'cube_root'
  []
  [Markers]
    [depth_frac]
      type = ErrorFractionMarker
      indicator = optical_depth
      refine = ${r_error_fraction}
      coarsen = 0.0
    []
    [rel_error]
      type = ValueThresholdMarker
      invert = true
      coarsen = ${r_stat_error}
      refine = ${c_stat_error}
      variable = heat_source_rel_error
      third_state = DO_NOTHING
    []
    [error_combo]
      type = BooleanComboMarker
      # Only refine iff the relative error is sufficiently low AND the optical depth is
      # sufficiently large.
      refine_markers = 'rel_error depth_frac'
      # Coarsen based exclusively on relative error.
      coarsen_markers = 'rel_error'
      boolean_operator = and
    []
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  particles = 20000
  inactive_batches = 500
  batches = 10000

  power = ${fparse 3000e6 / 273 * 4}

  normalize_by_global_tally = false
  assume_separate_tallies = true
  skip_statepoint = true

  [Tallies]
    [heat_source]
      type = MeshTally
      score = 'kappa_fission fission flux'
      name = 'heat_source fission flux'
      output = 'unrelaxed_tally_std_dev unrelaxed_tally_rel_error'
      blocks = 'uo2_center uo2'
    []
  []
[]

[Postprocessors]
  [num_elem]
    type = NumElements
    elem_filter = active
  []
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
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
  csv = true
[]
