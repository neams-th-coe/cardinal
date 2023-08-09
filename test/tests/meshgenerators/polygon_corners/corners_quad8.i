[Mesh]
  [fluid]
    type = FileMeshGenerator
    file = ../quad8_generator/fluid.exo
  []
  [rotate]
    type = TransformGenerator
    input = fluid
    transform = ROTATE
    vector_value = '30.0 0.0 0.0'
  []
  [to_quad8]
    type = NekMeshGenerator
    input = rotate
    boundaries_to_rebuild = '1 10000'
    geometry_type = cylinder

    curve_corners = true
    polygon_sides = 6
    polygon_size = 0.020207244566244058
    polygon_boundary = '10000'
    corner_radius = 0.008
  []

  parallel_type = replicated
[]
