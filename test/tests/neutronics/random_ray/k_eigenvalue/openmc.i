[Mesh]
  [initial]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -25.0
    xmax = 25.0
    ymin = -25.0
    ymax = 25.0
    zmin = -50.0
    zmax = 50.0
    nx = 10
    ny = 10
    nz = 10
  []
  [split]
    type = ParsedSubdomainMeshGenerator
    input = initial
    combinatorial_geometry = 'z < 0.0'
    block_id = '1'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  cell_level = 0

  power = 1.0
  source_rate_normalization = 'kappa_fission'

  batches = 100
  inactive_batches = 10
  particles = 1000

  [Tallies]
    [flux]
      type = CellTally
      score = 'flux'
      output = 'unrelaxed_tally_std_dev unrelaxed_tally_rel_error'
      normalize_by_global_tally = false
      filters = 'energy'
      block = '0 1'
    []
    [other]
      type = CellTally
      score = 'kappa_fission total fission nu_fission'
      output = 'unrelaxed_tally_std_dev unrelaxed_tally_rel_error'
      normalize_by_global_tally = false
      block = '0 1'
    []
  []
  [Filters/energy]
    type = EnergyFilter
    energy_boundaries = '1e-8 1e8'
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [k]
    type = KEigenvalue
    value_type = 'tracklength'
  []
  [k_std_dev]
    type = KEigenvalue
    value_type = 'tracklength'
    output = 'std_dev'
  []

  # Mean values of scores.
  [left_kappa_fission]
    type = PointValue
    variable = 'kappa_fission'
    point = '0.0 0.0 -25.0'
  []
  [right_kappa_fission]
    type = PointValue
    variable = 'kappa_fission'
    point = '0.0 0.0 25.0'
  []
  [left_flux]
    type = PointValue
    variable = 'flux_g1'
    point = '0.0 0.0 -25.0'
  []
  [right_flux]
    type = PointValue
    variable = 'flux_g1'
    point = '0.0 0.0 25.0'
  []
  [left_total]
    type = PointValue
    variable = 'total'
    point = '0.0 0.0 -25.0'
  []
  [right_total]
    type = PointValue
    variable = 'total'
    point = '0.0 0.0 25.0'
  []
  [left_fission]
    type = PointValue
    variable = 'fission'
    point = '0.0 0.0 -25.0'
  []
  [right_fission]
    type = PointValue
    variable = 'fission'
    point = '0.0 0.0 25.0'
  []
  [left_nu_fission]
    type = PointValue
    variable = 'nu_fission'
    point = '0.0 0.0 -25.0'
  []
  [right_nu_fission]
    type = PointValue
    variable = 'nu_fission'
    point = '0.0 0.0 25.0'
  []

  # Stndard deviation of the scores.
  [left_kappa_fission_std_dev]
    type = PointValue
    variable = 'kappa_fission_std_dev'
    point = '0.0 0.0 -25.0'
  []
  [right_kappa_fission_std_dev]
    type = PointValue
    variable = 'kappa_fission_std_dev'
    point = '0.0 0.0 25.0'
  []
  [left_flux_std_dev]
    type = PointValue
    variable = 'flux_g1_std_dev'
    point = '0.0 0.0 -25.0'
  []
  [right_flux_std_dev]
    type = PointValue
    variable = 'flux_g1_std_dev'
    point = '0.0 0.0 25.0'
  []
  [left_total_std_dev]
    type = PointValue
    variable = 'total_std_dev'
    point = '0.0 0.0 -25.0'
  []
  [right_total_std_dev]
    type = PointValue
    variable = 'total_std_dev'
    point = '0.0 0.0 25.0'
  []
  [left_fission_std_dev]
    type = PointValue
    variable = 'fission_std_dev'
    point = '0.0 0.0 -25.0'
  []
  [right_fission_std_dev]
    type = PointValue
    variable = 'fission_std_dev'
    point = '0.0 0.0 25.0'
  []
  [left_nu_fission_std_dev]
    type = PointValue
    variable = 'nu_fission_std_dev'
    point = '0.0 0.0 -25.0'
  []
  [right_nu_fission_std_dev]
    type = PointValue
    variable = 'nu_fission_std_dev'
    point = '0.0 0.0 25.0'
  []

  # Relative error of scores.
  [left_kappa_fission_rel_error]
    type = PointValue
    variable = 'kappa_fission_rel_error'
    point = '0.0 0.0 -25.0'
  []
  [right_kappa_fission_rel_error]
    type = PointValue
    variable = 'kappa_fission_rel_error'
    point = '0.0 0.0 25.0'
  []
  [left_flux_rel_error]
    type = PointValue
    variable = 'flux_g1_rel_error'
    point = '0.0 0.0 -25.0'
  []
  [right_flux_rel_error]
    type = PointValue
    variable = 'flux_g1_rel_error'
    point = '0.0 0.0 25.0'
  []
  [left_total_rel_error]
    type = PointValue
    variable = 'total_rel_error'
    point = '0.0 0.0 -25.0'
  []
  [right_total_rel_error]
    type = PointValue
    variable = 'total_rel_error'
    point = '0.0 0.0 25.0'
  []
  [left_fission_rel_error]
    type = PointValue
    variable = 'fission_rel_error'
    point = '0.0 0.0 -25.0'
  []
  [right_fission_rel_error]
    type = PointValue
    variable = 'fission_rel_error'
    point = '0.0 0.0 25.0'
  []
  [left_nu_fission_rel_error]
    type = PointValue
    variable = 'nu_fission_rel_error'
    point = '0.0 0.0 -25.0'
  []
  [right_nu_fission_rel_error]
    type = PointValue
    variable = 'nu_fission_rel_error'
    point = '0.0 0.0 25.0'
  []
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
