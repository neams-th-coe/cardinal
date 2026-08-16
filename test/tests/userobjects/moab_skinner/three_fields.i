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
  [temp]
    family = MONOMIAL
    order = CONSTANT
  []
  [rho]
    family = MONOMIAL
    order = CONSTANT
  []
  [zdep]
    family = MONOMIAL
    order = CONSTANT
  []
  [temp_bins]
    family = MONOMIAL
    order = CONSTANT
  []
  [rho_bins]
    family = MONOMIAL
    order = CONSTANT
  []
  [zdep_bins]
    family = MONOMIAL
    order = CONSTANT
  []
  [subdomain_bins]
    family = MONOMIAL
    order = CONSTANT
  []
  [bins]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [temp]
    type = FunctionAux
    variable = temp
    function = temp
    execute_on = timestep_begin
  []
  [rho]
    type = FunctionAux
    variable = rho
    function = rho
    execute_on = timestep_begin
  []
  [zdep]
    type = FunctionAux
    variable = zdep
    function = zdep
    execute_on = timestep_begin
  []
  [temp_bins]
    type = SkinnedBins
    variable = temp_bins
    skinner = moab
    skin_by = temp
  []
  [rho_bins]
    type = SkinnedBins
    variable = rho_bins
    skinner = moab
    skin_by = rho
  []
  [zdep_bins]
    type = SkinnedBins
    variable = zdep_bins
    skinner = moab
    skin_by = zdep
  []
  [subdomain_bins]
    type = SkinnedBins
    variable = subdomain_bins
    skinner = moab
    skin_by = subdomain
  []
  [bins]
    type = SkinnedBins
    variable = bins
    skinner = moab
  []
[]

[Functions]
  [temp]
    type = ParsedFunction
    expression = 400+x*100+100*t
  []
  [rho]
    type = ParsedFunction
    expression = 400+y*100+100*t
  []
  [zdep]
    type = ParsedFunction
    expression = 11+z*10+5*t
  []
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    fields = 'temp rho zdep'
    fields_min = '445 445 11'
    fields_max = '655 655 31'
    n_field_bins = '3 4 2'
    verbose = true
    material_blocks = "1 3"
    material_names = "mat mat"

  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  exodus = true
  hide = 'temp rho zdep'
[]