[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = -2
  xmax = 2
  ymin = -2
  ymax = 2
  zmin = -2
  zmax = 10
  nx = 5
  ny = 5
  nz = 5
[]

[ICs]
  [temp]
    type = ConstantIC
    variable = temp
    value = 1e5
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  temperature_blocks = '0'
  verbose = true
  cell_level = 0
[]

[Executioner]
  type = Transient
[]
