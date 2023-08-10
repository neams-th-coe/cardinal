[Mesh]
  [tube]
    type = FileMeshGenerator
    file = tube_in.e
  []
  [to_hex20]
    type = NekMeshGenerator
    input = tube
    boundary = '10'
    radius = '0.6'
    geometry_type = cylinder
  []

  parallel_type = replicated
[]
