[Mesh]
  [tube]
    type = FileMeshGenerator
    file = tube_in.e
  []
  [to_hex20]
    type = Hex20Generator
    input = tube
    boundary = 'rmax'
    radius = '0.25'
  []

  parallel_type = replicated
[]
