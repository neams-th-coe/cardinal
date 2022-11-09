[Mesh]
  [square]
    type = FileMeshGenerator
    file = square_in.e
  []
  [to_quad8]
    type = Quad8Generator
    input = square
    boundary = '10'
    radius = '0.6'
  []

  parallel_type = replicated
[]
