[Mesh]
  [box]
    type = AnnularMeshGenerator
    rmin = 0.1
    rmax = 0.2
    nr = 3
    nt = 8
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = box
    direction = '0 0 1'
    num_layers = 3
    heights = 3
  []

  second_order = true
[]
