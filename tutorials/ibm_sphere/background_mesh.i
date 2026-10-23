[Mesh]
  [cube]
    type = CartesianMeshGenerator
    dim = 3
    dx = '2 4 2'
    dy = '2 4 2'
    dz = '5 10 25'
    ix = '5 20 5'
    iy = '5 20 5'
    iz = '5 40 15'
  []
  [translate]
    type = TransformGenerator
    input = cube
    transform = translate
    vector_value = '-4 -4 -10'
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
