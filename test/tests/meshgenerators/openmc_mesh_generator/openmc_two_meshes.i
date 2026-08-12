[Mesh]
  [openmc_mesh_1]
    type = OpenMCMeshGenerator
    mesh_id = 1
    scaling = 10
  []
  [openmc_mesh_2]
    type = OpenMCMeshGenerator
    mesh_id = 5
    scaling = 10
  []
  [combined]
    type = CombinerGenerator
    inputs = "openmc_mesh_1 openmc_mesh_2"
  []
[]
