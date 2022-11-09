[Mesh]
  [tube]
    type = FileMeshGenerator
    file = tube_in.e
  []
  [to_hex20]
    type = NekMeshGenerator
    input = tube
    boundary = 'rmax'
    radius = '0.25'
  []

  parallel_type = replicated
[]
