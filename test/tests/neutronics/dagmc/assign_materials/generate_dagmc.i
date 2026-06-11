[Mesh]
  [slab]
    type = FileMeshGenerator
    file = ../mesh_tallies/slab.e
  []
  [block_1]
    type = ParsedSubdomainMeshGenerator
    input = slab
    combinatorial_geometry = 'x < 37.5'
    block_id = 1
    block_name = block_1
    epsilon = 1e-3
  []
  [block_2]
    type = ParsedSubdomainMeshGenerator
    input = block_1
    combinatorial_geometry = 'x >= 37.5'
    block_id = 2
    block_name = block_2
    epsilon = 1e-3
  []
  allow_renumbering = false
[]

[AuxVariables]
  [temp]
    order = CONSTANT
    family = MONOMIAL
    [AuxKernel]
        type = FunctionAux
        variable = temp
        function = 200
        execute_on = 'timestep_begin'
    []
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    temperature_min = 0
    temperature_max = 1000
    n_temperature_bins = 1
    temperature = temp
    build_graveyard = true
    graveyard_scale_inner = 1.1
    graveyard_scale_outer = 1.2
    material_blocks = 'block_1 block_2'
    material_names = '1 2'
    output_full = true
  []
[]

[Executioner]
  type = Steady
[]
