[Mesh]
  [fluid]
    type = FileMeshGenerator
    file = fluid.exo
  []
  [rotate]
    type = TransformGenerator
    input = fluid
    transform = rotate
    vector_value = '30.0 0.0 0.0'
  []
  [shift]
    type = TransformGenerator
    input = rotate
    transform = translate
    vector_value = '0.5 0.5 0.0'
  []
  [to_hex20]
    type = NekMeshGenerator
    input = shift
    boundaries_to_rebuild = '1 2 3 10000'
    geometry_type = cylinder

    curve_corners = true
    polygon_sides = 6
    polygon_size = ${fparse 0.03 / sqrt(3.0)}
    polygon_boundary = '10000'
    corner_radius = 0.005
    polygon_origins = '0.5 0.5 0.0'
  []

  parallel_type = replicated
[]
