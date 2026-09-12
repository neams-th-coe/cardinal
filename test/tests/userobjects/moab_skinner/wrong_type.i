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
  []
  [id2]
    type = ParsedSubdomainMeshGenerator
    input = id1
    combinatorial_geometry = 'z >= 0.0'
    block_id = 3
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[AuxVariables]
  [temp]
  []
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    fields = 'temp'
    fields_min = '445'
    fields_max = '655'
    n_field_bins = '3'
    material_names = "mat mat"

  []
[]

[Executioner]
  type = Transient
[]
