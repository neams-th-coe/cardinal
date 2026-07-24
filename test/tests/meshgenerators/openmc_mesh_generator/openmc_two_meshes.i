[Mesh]
  [openmc_mesh]
    type = OpenMCMeshGenerator
    mesh_id = 1
    scaling = 10
  []
  [openmc_mesh_2]
    type = OpenMCMeshGenerator
    mesh_id = 1
    scaling = 10
  []
  final_generator = openmc_mesh_2
[]
