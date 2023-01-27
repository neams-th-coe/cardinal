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
    verbose = true
    material_names = "mat mat"

    temperature = temp
    n_temperature_bins = 3
    temperature_min = 445.0
    temperature_max = 655.0

    output_full = true
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
