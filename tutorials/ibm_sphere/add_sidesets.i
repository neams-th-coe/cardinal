[Mesh]
  [file]
    type = FileMeshGenerator
    file = filtered_mesh.exo
  []
  [inlet]
    type = SideSetsFromNormalsGenerator
    input = file
    new_boundary = '1'
    normals = '0 0 -1'
  []
  [outlet]
    type = SideSetsFromNormalsGenerator
    input = inlet
    new_boundary = '2'
    normals = '0 0 1'
  []
  [side1]
    type = SideSetsFromNormalsGenerator
    input = outlet
    new_boundary = '3'
    normals = '1 0 0'
  []
  [side2]
    type = SideSetsFromNormalsGenerator
    input = side1
    new_boundary = '3'
    normals = '-1 0 0'
  []
  [side3]
    type = SideSetsFromNormalsGenerator
    input = side2
    new_boundary = '3'
    normals = '0 1 0'
  []
  [side4]
    type = SideSetsFromNormalsGenerator
    input = side3
    new_boundary = '3'
    normals = '0 -1 0'
  []
  [around_sphere]
    type = SideSetsFromBoundingBoxGenerator
    input = side4
    bottom_left = '-2 -2 -2'
    top_right = '2 2 2'
    boundary_new = 3
    include_only_external_sides = true
    replace = true
  []
  [to_hex20]
    type = ElementOrderConversionGenerator
    input = around_sphere
    conversion_type = SECOND_ORDER_NONFULL
  []
[]

# the remaining part of this file only exists in order to obtain
# data that can be used to generate a regression test
[Problem]
  type = FEProblem
  solve = false
[]

[Postprocessors]
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
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
