[Mesh]
  [initial]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -25.0
    xmax = 25.0
    ymin = -25.0
    ymax = 25.0
    zmin = -50.0
    zmax = 50.0
    nx = 1
    ny = 1
    nz = 10
  []
  [split]
    type = ParsedSubdomainMeshGenerator
    input = initial
    combinatorial_geometry = 'z < 0.0'
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
    expression = '500.0 + 300.0 * (z + 50.0) / 100.0'
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

  batches = 100
  inactive_batches = 10
  particles = 1000
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [k]
    type = KEigenvalue
    value_type = 'tracklength'
  []
  [T_left]
    type = PointValue
    variable = cell_temp
    point = '0.0 0.0 -25.0'
  []
  [T_right]
    type = PointValue
    variable = cell_temp
    point = '0.0 0.0 25.0'
  []
[]

[Outputs]
  csv = true
[]
