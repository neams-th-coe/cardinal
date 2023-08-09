[Mesh]
  [box]
    type = FileMeshGenerator
    file = box_in.e
  []
  [exterior]
    type = SideSetsAroundSubdomainGenerator
    input = box
    block = 0
    normal = '1 0 0'
    new_boundary = 50
  []
  [to_hex20]
    type = NekMeshGenerator
    input = exterior
    boundary = '1 50'
    radius = '2.0 2.0'
    geometry_type = cylinder
  []

  parallel_type = replicated
[]
