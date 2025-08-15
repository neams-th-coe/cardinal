[Mesh]
  [g]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 10
    xmax = 20
    ymin = 0
    ymax = 10
    zmin = 0
    zmax = 10
    nx = 10
    ny = 1
    nz = 1
  []
[]

[ICs]
  [density]
    type = ConstantIC
    variable = density
    value = 1900.0
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  density_blocks = '0'
  cell_level = 0
  verbose = true

  [CriticalitySearch]
    type = BoratedWater
    absent_nuclides = 'O18'
    material_id = 2
    minimum = 0
    maximum = 1000
    tolerance = 5e-2
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
