[Mesh]
  [tube]
    type = FileMeshGenerator
    file = tube_in.e
  []
  [to_quad8]
    type = Quad8Generator
    input = tube
    boundary = 'rmin'
    radius = '0.05'
  []

  parallel_type = replicated
[]
