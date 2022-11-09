[Mesh]
  [tube]
    type = FileMeshGenerator
    file = tube_in.e
  []
  [to_quad8]
    type = NekMeshGenerator
    input = tube
  []

  parallel_type = replicated
[]
