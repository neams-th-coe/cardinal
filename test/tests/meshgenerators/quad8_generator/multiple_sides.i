[Mesh]
  [square]
    type = FileMeshGenerator
    file = square_in.e
  []
  [to_quad8]
    type = NekMeshGenerator
    input = square
    boundary = '1 2'
    radius = '2.0 2.0'
    geometry_type = cylinder
  []

  parallel_type = replicated
[]
