!include openmc.i

[Mesh]
  [annulus]
    radii := '${r_inner}'
    rings := '1'
  []
  [extrude]
    subdomain_swaps := '1 1'
  []
[]

[Problem]
  power = 1.0
  lowest_cell_level = 1

  [Tallies/cell]
    type = CellTally
    # Only tallying the drum which results in lost hits over the domain.
    check_tally_sum = false
    normalize_by_global_tally = false
  []
[]

[Postprocessors]
  [point_cell_id]
    type = PointValue
    variable = cell_id
    point = '-8.0 1.0 50.0'
  []
[]

[Outputs]
  hide := 'k k_residual critical_value k_converged_within_tolerance'
  execute_on = 'TIMESTEP_END'
[]

