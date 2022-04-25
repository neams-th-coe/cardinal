[Mesh]
  [fluid]
    type = FileMeshGenerator
    file = fluid.exo
  []
  [to_hex20]
    type = Hex20Generator
    input = fluid
    boundary = '1'
    radius = '${fparse 7.646e-3 / 2.0}'
    origin = '+0.00000000 +0.00000000 +0.00000000
              +0.00000000 +0.00896570 +0.00000000
              -0.00776452 +0.00448285 +0.00000000
              -0.00776452 -0.00448285 +0.00000000
              +0.00000000 -0.00896570 +0.00000000
              +0.00776452 -0.00448285 +0.00000000
              +0.00776452 +0.00448285 +0.00000000'
    boundary_to_rebuild = '1 2 3'
  []

  parallel_type = replicated
[]
