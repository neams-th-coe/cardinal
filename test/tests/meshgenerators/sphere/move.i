[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = sphere_in.e
  []
  [shift]
    type = TransformGenerator
    input = sphere
    transform = translate
    vector_value = '1.0 2.0 3.0'
  []
  [move]
    type = NekMeshGenerator
    input = shift
    geometry_type = sphere
    origins = '1.0 2.0 3.0'
    radius = 0.6
    boundary = '0'
  []
[]
