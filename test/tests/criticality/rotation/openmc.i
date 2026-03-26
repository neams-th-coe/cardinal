!include common_input.i

[Mesh]
  [annulus]
    type = ConcentricCircleMeshGenerator
    radii = '${r_inner} ${r_outer}'
    rings = '1 1'
    num_sectors = '4'
    has_outer_square = 'False'
    preserve_volumes = 'False'
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = 'annulus'
    heights = '${height}'
    num_layers = '1'
    direction = '0 0 1'
    subdomain_swaps = '2 1'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem

  [CriticalitySearch]
    type = RotationSearch
    cell_ids = '5'
    rotation_axis = 'z'
    minimum = '0.0'
    maximum = '90.0'
    tolerance = 1e-2
  []
[]

[Executioner]
  type = Steady
[]

# This test will output a 1 to CSV if the timestep converges a criticality search
[Postprocessors]
  [k]
    type = KEigenvalue
  []
  [k_residual]
    type = ParsedPostprocessor
    expression = 'abs(k - 1.0)'
    pp_names = 'k'
  []
  [k_converged_within_tolerance]
    type = ParsedPostprocessor
    expression = 'if (k_residual < 1e-2, 1, 0)'
    pp_names = 'k_residual'
  []
[]

[Outputs]
  csv = true
  hide = 'k k_residual critical_value'
[]

