[Mesh]
  [square]
    type = FileMeshGenerator
    file = square_in.e
  []

  # this translates the mesh so that its lower left corner is at (0, 0, 0)
  [offcenter]
    type = TransformGenerator
    input = square
    transform = TRANSLATE
    vector_value = '0.5 0.5 0.0'
  []
  [to_quad8]
    type = NekMeshGenerator
    input = offcenter
    boundary = '2'
    radius = '2.0'
    geometry_type = cylinder
  []

  parallel_type = replicated
[]
