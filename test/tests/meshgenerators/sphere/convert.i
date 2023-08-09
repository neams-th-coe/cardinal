[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = sphere_in.e
  []
  [move]
    type = NekMeshGenerator
    input = sphere
    geometry_type = sphere
    radius = 0.6
    boundary = '0'
  []
[]
