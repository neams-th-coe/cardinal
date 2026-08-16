[Mesh]
  [cube]
    type = FileMeshGenerator
    file = ../../../neutronics/meshes/tet_cube.e
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
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 500.0
  []
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    fields = 'temp'
    fields_min = '445'
    fields_max = '655'
    n_field_bins = '3'
    verbose = true
    material_blocks = "1 3"
    material_names = "mat mat"


    output_full = true
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
