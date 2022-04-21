[Mesh]
  [box]
    type = FileMeshGenerator
    file = box_in.e
  []
  [offcenter]
    type = TransformGenerator
    input = box
    transform = TRANSLATE
    vector_value = '1.0 2.0 3.0'
  []
  [to_hex20]
    type = SecondOrderHexGenerator
    input = offcenter
    boundary = '1 3'
    radius = 2.0
  []

  parallel_type = replicated
[]
