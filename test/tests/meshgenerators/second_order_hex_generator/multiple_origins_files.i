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
    origins_files = "origins_file.txt"
    boundary_to_rebuild = '1 2 3'
  []

  parallel_type = replicated
[]
