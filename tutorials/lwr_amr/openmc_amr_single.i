[Mesh]
  [file]
    type = FileMeshGenerator
    file = mesh_pin_in.e
  []
[]

[Adaptivity]
  marker = error_combo
  steps = 2

  [Indicators/optical_depth]
    type = ElementOpticalDepthIndicator
    rxn_rate = 'fission'
    h_type = 'max'
  []
  [Markers]
    [depth_frac]
      type = ErrorFractionMarker
      indicator = optical_depth
      refine = 0.3
      coarsen = 0.0
    []
    [rel_error]
      type = ValueThresholdMarker
      invert = true
      coarsen = 1e-1
      refine = 5e-2
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
  particles = 1000
  inactive_batches = 50
  batches = 1000

  power = ${fparse 3000e6 / 273 * 4}
  source_rate_normalization = 'kappa_fission'

  assume_separate_tallies = true
  skip_statepoint = true

  [Tallies]
    [heat_source]
      type = MeshTally
      score = 'kappa_fission fission flux'
      name = 'heat_source fission flux'
      output = 'unrelaxed_tally_std_dev unrelaxed_tally_rel_error'
      block = 'uo2_center uo2'

      normalize_by_global_tally = false
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
