[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = -50.0
  xmax = 50.0
  ymin = -25.0
  ymax = 25.0
  zmin = 0.0
  zmax = 50.0
  nx = 15
  ny = 15
  nz = 15
[]

[ICs]
  [temp]
    type = ConstantIC
    variable = temp
    value = 500.0
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  temperature_blocks = '0'
  cell_level = 0
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
