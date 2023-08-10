[Mesh]
  [fluid]
    type = FileMeshGenerator
    file = with_bl.exo
  []
  [rotate]
    type = TransformGenerator
    input = fluid
    transform = rotate
    vector_value = '30.0 0.0 0.0'
  []
  [to_hex20]
    type = NekMeshGenerator
    input = rotate
    boundaries_to_rebuild = '1 2 3 4'
    geometry_type = cylinder

    curve_corners = true
    polygon_sides = 6
    polygon_size = 0.018001405522227287
    polygon_boundary = '4'
    polygon_layers = 3
    corner_radius = 0.002
  []

  parallel_type = replicated
[]
