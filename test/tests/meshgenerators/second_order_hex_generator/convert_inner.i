[Mesh]
  [tube]
    type = FileMeshGenerator
    file = tube_in.e
  []
  [to_hex20]
    type = NekMeshGenerator
    input = tube
    boundary = 'rmin'
    radius = '0.05'
    geometry_type = cylinder
  []

  parallel_type = replicated
[]
