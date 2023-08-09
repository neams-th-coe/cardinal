[Mesh]
  [fluid]
    type = FileMeshGenerator
    file = fluid_with_multiple_bl.exo
  []
  [to_hex20]
    type = NekMeshGenerator
    input = fluid
    boundary = '1'
    radius = '${fparse 7.646e-3 / 2.0}'
    origins = '+0.00000000 +0.00000000 +0.00000000
               +0.00000000 +0.00896570 +0.00000000
               -0.00776452 +0.00448285 +0.00000000
               -0.00776452 -0.00448285 +0.00000000
               +0.00000000 -0.00896570 +0.00000000
               +0.00776452 -0.00448285 +0.00000000
               +0.00776452 +0.00448285 +0.00000000'
    layers = '3'
    geometry_type = cylinder
  []

  parallel_type = replicated
[]
