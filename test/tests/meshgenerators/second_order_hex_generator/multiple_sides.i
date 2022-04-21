[Mesh]
  [box]
    type = FileMeshGenerator
    file = box_in.e
  []
  [to_hex20]
    type = SecondOrderHexGenerator
    input = box
    boundary = '1 2'
    radius = 2.0
  []
[]
