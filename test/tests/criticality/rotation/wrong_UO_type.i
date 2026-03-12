!include common_input.i

[Mesh]
  [annulus]
    type = ConcentricCircleMeshGenerator
    radii = '${r_inner} ${r_outer}'
    rings = '2 2'
    num_sectors = '12'
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

[AuxVariables]
  [z]
    # variable to give to the UO
  []
[]

[Problem]
  type = OpenMCCellAverageProblem

  [CriticalitySearch]
    type = RotationSearch
    transform_name = 'wrong_uo_type'
    rotation_axis = 'z'
    minimum = '0.0'
    maximum = '90.0'
    tolerance = 1e-2
  []
[]

[UserObjects]
  [wrong_uo_type]
    type = LayeredAverage
    variable = 'z'
    num_layers = '1'
    direction = 'z'
  []
[]

[Executioner]
  type = Steady
[]

# This test will output a 1 to CSV if the timestep converges a criticality search
[Postprocessors]
  [drum_angle]
    type = ConstantPostprocessor
    value = '90'
  []
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

