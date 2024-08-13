[Mesh]
  [m]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 1
    nz = 1
    xmax = 10
    ymax = 10
    zmax = 10
  []
[]

[AuxVariables]
  [cell_temperature]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_temperature]
    type = CellTemperatureAux
    variable = cell_temperature
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  temperature_blocks = '0'
  cell_level = 0
[]

[ICs]
  [temp]
    type = FunctionIC
    variable = temp
    function = temp
  []
[]

[Functions]
  [temp]
    type = ParsedFunction
    expression = '500+50*x'
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [tl]
    type = PointValue
    variable = cell_temperature
    point = '2.5 5 5'
  []
  [tr]
    type = PointValue
    variable = cell_temperature
    point = '7.5 5 5'
  []
  [k]
    type = KEigenvalue
  []
[]

[Outputs]
  csv = true
[]
