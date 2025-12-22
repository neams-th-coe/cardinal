[Mesh]
  [g]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 10
    xmax = 20
    ymin = 0
    ymax = 10
    zmin = 0
    zmax = 10
    nx = 10
    ny = 1
    nz = 1
  []
[]

[ICs]
  [density]
    type = ConstantIC
    variable = density
    value = 1900.0
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  density_blocks = '0'
  cell_level = 0
  verbose = true

  [CriticalitySearch]
    type = BoratedWater
    absent_nuclides = 'O18'
    material_id = 2
    minimum = 0
    maximum = 1000
    tolerance = 1e-2
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
