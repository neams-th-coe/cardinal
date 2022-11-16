[Mesh]
  [tube]
    type = FileMeshGenerator
    file = tube_in.e
  []
  [to_hex20]
    type = NekMeshGenerator
    input = tube
    boundaries_to_rebuild = 'rmax 3'
  []

  parallel_type = replicated
[]
