l = 0.2
h = 1.0

[Mesh]
  [solid]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = ${fparse -l / 2.0}
    xmax = ${fparse l / 2.0}
    ymin = ${fparse -l / 2.0}
    ymax = ${fparse l / 2.0}
    zmin = 0.0
    zmax = ${h}
    nx = 4
    ny = 4
    nz = 20
  []
[]

[AuxVariables]
  [cell_material_id]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_material_id]
    type = CellMaterialIDAux
    variable = cell_material_id
  []
[]

[ICs]
  [density]
    type = FunctionIC
    variable = density
    function = density
  []
[]

[Functions]
  [density]
    type = ParsedFunction
    expression = 450.0
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  scaling = 100.0
  density_blocks = '0'
  cell_level = 0
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [material_left]
    type = PointValue
    variable = cell_material_id
    point = '${fparse -l/4} 0 ${fparse h/2}'
  []
[]

[Outputs]
  csv = true
[]
