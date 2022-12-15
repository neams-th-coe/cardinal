[Mesh]
  [cube]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 5
    ny = 5
    nz = 20
    xmin = -4.0
    xmax = 4.0
    ymin = -4.0
    ymax = 4.0
    zmin = -4.0
    zmax = 4.0
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  tally_type = mesh
  solid_cell_level = 0
  solid_blocks = '0'
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
