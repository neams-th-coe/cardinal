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

[Problem]
  type = OpenMCCellAverageProblem
  temperature_blocks = '0'
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
