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
  []

  parallel_type = replicated
[]
