[Mesh]
  [tube]
    type = FileMeshGenerator
    file = tube_in.e
  []
  [to_hex20]
    type = Hex20Generator
    input = tube
    boundaries_to_rebuild = ''
  []

  parallel_type = replicated
[]
