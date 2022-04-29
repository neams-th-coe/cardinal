[Mesh]
  [tube]
    type = FileMeshGenerator
    file = tube_in.e
  []
  [to_hex20]
    type = Hex20Generator
    input = tube
    boundary = 'rmin'
    radius = '0.05'
  []

  parallel_type = replicated
[]
