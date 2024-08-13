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

[Problem]
  type = OpenMCCellAverageProblem
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
