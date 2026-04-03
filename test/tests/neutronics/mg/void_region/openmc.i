[Mesh]
  type = GeneratedMesh
  dim = 3

  xmin = -50.0
  xmax =  50.0
  ymin =  -1.0
  ymax =   1.0
  zmin =  -1.0
  zmax =   1.0

  nx = 1
  ny = 1
  nz = 1
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  temperature_blocks = '0'
  cell_level = 0
[]

[ICs]
  [temp]
    type = ConstantIC
    variable = temp
    value = 293
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [k]
    type = KEigenvalue
  []
[]

[Outputs]
  csv = true
[]
