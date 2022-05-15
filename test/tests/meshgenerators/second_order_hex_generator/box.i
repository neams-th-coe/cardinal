[Mesh]
  [box]
    type = CartesianMeshGenerator
    ix = 3
    iy = 3
    iz = 3
    dx = 1.0
    dy = 1.0
    dz = 2.0
    dim = 3
  []
  [translate]
    type = TransformGenerator
    input = box
    transform = TRANSLATE_CENTER_ORIGIN
  []

  second_order = true
[]
