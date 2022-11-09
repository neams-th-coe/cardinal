[Mesh]
  [tube]
    type = FileMeshGenerator
    file = tube_in.e
  []
  [to_quad8]
    type = Quad8Generator
    input = tube
    boundary = 'rmax'
    radius = '0.25'
  []

  parallel_type = replicated
[]
