[Mesh]
  [tube]
    type = FileMeshGenerator
    file = tube_in.e
  []
  [to_hex20]
    type = SecondOrderHexGenerator
    input = tube
  []
[]
