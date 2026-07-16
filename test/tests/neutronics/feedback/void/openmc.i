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
  [add_block]
    type = ParsedSubdomainMeshGenerator
    input = solid
    combinatorial_geometry = 'z < ${fparse h / 2.0}'
    block_id = 100
  []
[]

[AuxVariables]
  [cell_density]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_density]
    type = CellDensityAux
    variable = cell_density
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
  density_blocks = '0 100'
  cell_level = 1
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [density_left]
    type = PointValue
    variable = cell_density
    point = '${fparse -l/4} 0 ${fparse h/2}'
  []
  [density_right_void]
    type = PointValue
    variable = cell_density
    point = '${fparse l/4} 0 ${fparse h/2}'
  []
[]

[Outputs]
  csv = true
[]
