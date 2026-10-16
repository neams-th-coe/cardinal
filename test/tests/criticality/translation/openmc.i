r_inner = 10.0
r_outer = 20.0
height = 100.0

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
  power = 1.0
  lowest_cell_level = 1
  particles = 5000
  [Tallies/cell]
    type = CellTally
  []
  [CriticalitySearch]
    type = TranslationSearch
    cell_ids = '5'
    translation_axis = 'x'
    minimum = '4.4'
    maximum = '4.7'
    root_tol = '1e-2'
    k_tol = '1e-2'
  []
[]

[Executioner]
  type = Steady
[]

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
