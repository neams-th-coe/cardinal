r_pin = ${fparse 18 / 2.0}
L = 40.0

[Mesh]
  [fuel] # this makes a circle that will represent the fuel
    type = AnnularMeshGenerator
    nr = 9
    nt = 20
    rmin = 0
    rmax = ${r_pin}
    quad_subdomain_id = 2
    tri_subdomain_id = 3
  []
  [extrude] # this extrudes the circle in the axial direction
    type = AdvancedExtruderGenerator
    input = fuel
    heights = '${L}'
    num_layers = '10'
    direction = '0 0 1'
  []
  [shift]
    type = TransformGenerator
    input = extrude
    transform = translate
    vector_value = '0.0 0.0 ${fparse -1.0 * L / 2.0}'
  []
[]
