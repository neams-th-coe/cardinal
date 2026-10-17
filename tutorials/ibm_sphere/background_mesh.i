[Mesh]
  [cube]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 20
    ny = 20
    nz = 100
    xmin = -4.0
    xmax = 4.0
    ymin = -4.0
    ymax = 4.0
    zmin = -10.0
    zmax = 30.0
  []
[]

# these parts that follow are only for creating a regression test to ensure
# tutorial stays up to date
[Problem]
  solve = false
  type = FEProblem
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [area_0]
    type = AreaPostprocessor
    boundary = '0'
  []
  [area_1]
    type = AreaPostprocessor
    boundary = '1'
  []
  [area_2]
    type = AreaPostprocessor
    boundary = '2'
  []
  [area_3]
    type = AreaPostprocessor
    boundary = '3'
  []
  [area_4]
    type = AreaPostprocessor
    boundary = '4'
  []
  [area_5]
    type = AreaPostprocessor
    boundary = '5'
  []
[]

[Outputs]
  csv = true
[]
