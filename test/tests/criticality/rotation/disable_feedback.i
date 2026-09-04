!include openmc.i

[Mesh]
  [annulus]
    radii := '${r_inner}'
    rings := '1'
  []
  [extrude]
    subdomain_swaps := '1 1'
  []
  [half_block_2]
    type = ParsedSubdomainMeshGenerator
    input = extrude
    combinatorial_geometry = 'y >= 0.0'
    block_id = 2
    epsilon = 1e-8
  []
[]

[AuxVariables]
  [cell_temp]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_temp]
    type = CellTemperatureAux
    variable = cell_temp
  []
[]

[ICs]
  [high]
    type = ConstantIC
    variable = temp
    block = 1
    value = 600
  []
  [low]
    type = ConstantIC
    variable = temp
    block = 2
    value = 300
  []
[]

[Problem]
  lowest_cell_level = 1
  temperature_blocks = '1 2'

  [CriticalitySearch]
    apply_feedback_in_search = false
  []
[]

[Postprocessors]
  [point_cell_temp]
    type = PointValue
    variable = cell_temp
    point = '-8.0 1.0 50.0'
  []
[]

[Outputs]
  hide := 'k k_residual critical_value k_converged_within_tolerance'
  execute_on = 'TIMESTEP_END'
[]

