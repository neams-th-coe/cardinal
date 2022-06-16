[Mesh]
  [fluid]
    type = FileMeshGenerator
    file = fluid.exo
  []
  [to_hex20]
    type = Hex20Generator
    input = fluid
    boundary_to_rebuild = '1 2 3'

    curve_corners = true
    polygon_sides = 6
    polygon_size = 0.015
  []

  parallel_type = replicated
[]
