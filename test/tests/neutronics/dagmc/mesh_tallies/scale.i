scale = 100.0

[Mesh]
  [file]
    type = FileMeshGenerator
    file = slab.e
  []
  [scale]
    type = TransformGenerator
    input = file
    transform = scale
    vector_value = '${fparse 1.0 / scale} ${fparse 1.0 / scale} ${fparse 1.0 / scale}'
  []
[]
