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
    nz = 11
  []
  [split1]
    type = ParsedSubdomainMeshGenerator
    input = initial
    combinatorial_geometry = 'z >= -5.0 & z <= 5.0'
    block_id = '1'
  []
  [split2]
    type = ParsedSubdomainMeshGenerator
    input = split1
    combinatorial_geometry = 'z > 5.0'
    block_id = '2'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  cell_level = 0

  source_strength = 1e8

  inactive_batches = 10
  batches = 100
  particles = 1000

  [Tallies]
    [flux]
      type = CellTally
      score = 'flux'
      output = 'unrelaxed_tally_std_dev unrelaxed_tally_rel_error'
      normalize_by_global_tally = false
      check_tally_sum = false
      filters = 'energy'
      block = '0 1 2'
    []
    [other]
      type = CellTally
      score = 'total'
      output = 'unrelaxed_tally_std_dev unrelaxed_tally_rel_error'
      normalize_by_global_tally = false
      check_tally_sum = false
      block = '0 1 2'
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
  # Mean values of scores.
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

  # Stndard deviation of the scores.
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

  # Relative error of scores.
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
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
