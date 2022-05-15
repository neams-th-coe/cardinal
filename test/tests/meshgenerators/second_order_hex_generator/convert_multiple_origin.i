[Mesh]
  [tube]
    type = FileMeshGenerator
    file = tube_in.e
  []
  [translate]
    type = TransformGenerator
    input = tube
    transform = TRANSLATE
    vector_value = '1.0 0.0 0.0'
  []
  [to_hex20]
    type = Hex20Generator
    input = translate
    boundary = 'rmin rmax'
    radius = '0.05 0.2'
    origins = '1.0 0.0 0.0;
               1.0 0.0 0.0'
  []

  parallel_type = replicated
[]
