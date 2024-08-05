[Mesh]
  [initial]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -50.0
    xmax = 50.0
    ymin = -25.0
    ymax = 25.0
    zmin = 0.0
    zmax = 50.0
    nx = 20
    ny = 15
    nz = 15
  []
  [split]
    type = ParsedSubdomainMeshGenerator
    input = initial
    combinatorial_geometry = 'x < 0.0'
    block_id = '3'
  []
[]

[ICs]
  [temp]
    type = FunctionIC
    variable = temp
    function = temperature
  []
[]

[Functions]
  [temperature]
    type = ParsedFunction
    expression = '1000 + 10*x'
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

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  cell_level = 0
  temperature_blocks = '3 0'
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [k]
    type = KEigenvalue
  []
  [T_left]
    type = PointValue
    variable = cell_temp
    point = '-25.0 0.0 0.0'
  []
  [T_right]
    type = PointValue
    variable = cell_temp
    point = '25.0 0.0 0.0'
  []
[]

[Outputs]
  csv = true
[]
