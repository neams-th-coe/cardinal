[Mesh]
  [square]
    type = FileMeshGenerator
    file = square_in.e
  []
  [exterior]
    type = SideSetsAroundSubdomainGenerator
    input = square
    block = 0
    normal = '1 0 0'
    new_boundary = 50
  []
  [to_quad8]
    type = Quad8Generator
    input = exterior
    boundary = '1 50'
    radius = '2.0 2.0'
  []

  parallel_type = replicated
[]
