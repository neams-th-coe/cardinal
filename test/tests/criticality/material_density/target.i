[Mesh]
  [g]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 0
    xmax = 10
    ymin = 0
    ymax = 10
    zmin = 0
    zmax = 10
  []
[]

[Problem]
  type = OpenMCCellAverageProblem

  [CriticalitySearch]
    type = OpenMCMaterialDensity
    material_id = 1
    minimum = 10000
    maximum = 20000
    tolerance = 1e-2
    target = 0.9
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
    expression = 'abs(k - 0.9)'
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
