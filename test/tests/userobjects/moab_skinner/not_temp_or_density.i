[Mesh]
  [cube]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/tet_cube.e
  []
  [id1]
    type = ParsedSubdomainMeshGenerator
    input = cube
    combinatorial_geometry = 'z < 0.0'
    block_id = 1
    epsilon = 1e-8
  []
  [id2]
    type = ParsedSubdomainMeshGenerator
    input = id1
    combinatorial_geometry = 'z >= 0.0'
    block_id = 3
    epsilon = 1e-8
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    verbose = true
    material_blocks = "1 3"
    material_names = "mat mat"
    output_skins = true
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
[]
