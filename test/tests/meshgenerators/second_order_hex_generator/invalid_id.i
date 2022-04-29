[Mesh]
  [tube]
    type = FileMeshGenerator
    file = tube_in.e
  []
  [to_hex20]
    type = Hex20Generator
    input = tube
    boundary = '10'
    radius = '0.6'
  []

  parallel_type = replicated
[]
