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
    xml_directory = "./gold/"
  []
  final_generator = openmc_mesh_2
[]
