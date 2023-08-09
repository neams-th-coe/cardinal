[Mesh]
  [box]
    type = FileMeshGenerator
    file = box_in.e
  []
  [to_hex20]
    type = NekMeshGenerator
    input = box
    boundary = '1 2'
    radius = '2.0 2.0'
    geometry_type = cylinder
  []

  parallel_type = replicated
[]
