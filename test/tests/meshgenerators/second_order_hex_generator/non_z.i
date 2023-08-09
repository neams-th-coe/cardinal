[Mesh]
  [tube]
    type = FileMeshGenerator
    file = tube_in.e
  []
  [rotate]
    type = TransformGenerator
    input = tube
    transform = rotate
    vector_value = '0.0 90.0 0.0'
  []
  [to_hex20]
    type = NekMeshGenerator
    input = rotate
    boundary = 'rmax'
    radius = '0.25'
    axis = y
    geometry_type = cylinder
  []

  parallel_type = replicated
[]
