[Mesh]
  [fluid]
    type = FileMeshGenerator
    file = fluid.exo
  []
  [to_hex20]
    type = NekMeshGenerator
    input = fluid
    boundary = '1'
    radius = '${fparse 7.646e-3 / 2.0}'
    origins_files = "origins_file.txt"
    geometry_type = cylinder
  []

  parallel_type = replicated
[]
