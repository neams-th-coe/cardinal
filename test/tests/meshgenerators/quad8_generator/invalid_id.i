[Mesh]
  [square]
    type = FileMeshGenerator
    file = square_in.e
  []
  [to_quad8]
    type = NekMeshGenerator
    input = square
    boundary = '10'
    radius = '0.6'
    geometry_type = cylinder
  []

  parallel_type = replicated
[]
