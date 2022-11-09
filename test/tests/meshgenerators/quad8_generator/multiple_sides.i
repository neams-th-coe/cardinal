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
  []

  parallel_type = replicated
[]
