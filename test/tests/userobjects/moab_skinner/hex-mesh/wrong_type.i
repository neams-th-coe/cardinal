[Mesh]
  [cube]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -0.5
    xmax = 0.5
    ymin = -0.5
    ymax = 0.5
    zmin = -0.5
    zmax = 0.5
    nx = 5
    ny = 5
    nz = 5
    elem_type = HEX20
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
  []
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    material_names = "mat mat"

    temperature = temp
    n_temperature_bins = 3
    temperature_min = 445.0
    temperature_max = 655.0
  []
[]

[Executioner]
  type = Transient
[]
