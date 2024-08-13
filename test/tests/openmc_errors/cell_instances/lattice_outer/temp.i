[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = -1
  xmax = 2
  ymin = -1
  ymax = 2
  zmin = -1
  zmax = 1
  nx = 20
  ny = 20
  nz = 20
[]

[ICs]
  [temp]
    type = ConstantIC
    variable = temp
    value = 500
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 1.0
  temperature_blocks = 0
  cell_level = 1
[]

[Executioner]
  type = Steady
[]
