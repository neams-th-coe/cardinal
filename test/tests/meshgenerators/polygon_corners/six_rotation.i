[Mesh]
  [fluid]
    type = FileMeshGenerator
    file = fluid.exo
  []
  [to_hex20]
    type = NekMeshGenerator
    input = fluid
    boundaries_to_rebuild = '1 2 3 10000'
    geometry_type = cylinder

    curve_corners = true
    polygon_sides = 6
    polygon_size = ${fparse 0.03 / sqrt(3.0)}
    polygon_boundary = '10000'
    corner_radius = 0.01
    rotation_angle = 30.0
  []

  parallel_type = replicated
[]
