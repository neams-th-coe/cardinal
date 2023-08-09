[Mesh]
  [fluid]
    type = FileMeshGenerator
    file = fluid.exo
  []
  [to_quad8]
    type = NekMeshGenerator
    input = fluid
    boundary = '1'
    layers = 2
    radius = '${fparse 7.55e-3 / 2.0}'
    origins = '+0.00000000 +0.00000000 +0.00000000
               +0.00000000 +0.00890900 +0.00000000
               -0.00771542 +0.00445450 +0.00000000
               -0.00771542 -0.00445450 +0.00000000
               +0.00000000 -0.00890900 +0.00000000
               +0.00771542 -0.00445450 +0.00000000
               +0.00771542 +0.00445450 +0.00000000'
    geometry_type = cylinder
  []

  parallel_type = replicated
[]
