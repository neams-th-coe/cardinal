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

[AuxVariables]
  [eta]
    family = MONOMIAL
    order = constant
  []
  [eta_bins]
    family = MONOMIAL
    order = CONSTANT
  []  
[]

[AuxKernels]
  [eta]
    type = FunctionAux
    variable = eta
    function = eta_fn
    execute_on = timestep_begin
  []
  [eta_bins]
    type = SkinnedBins
    variable = eta_bins
    skinner = moab
    skin_by = eta
  []
[]

[Functions]
  [eta_fn]
    type = ParsedFunction
    expression = 400+x*100+100*t
  []
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    fields = 'eta'
    fields_min = '445'
    fields_max = '655'
    n_field_bins = 3
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
  exodus = true
[]
