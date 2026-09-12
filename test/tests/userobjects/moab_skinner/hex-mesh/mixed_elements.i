[Mesh]
  [cylinder]
    type = AnnularMeshGenerator
    nr = 10
    nt = 20
    rmin = 0
    rmax = 1
    quad_subdomain_id = 2
    tri_subdomain_id = 3
    growth_r = -1.2
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = cylinder
    heights = '10'
    num_layers = '10'
    direction = '0 0 1'
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[AuxVariables]
  [rho]
    family = MONOMIAL
    order = CONSTANT
  []
  [temp]
    family = MONOMIAL
    order = CONSTANT
  []
  [rho_bins]
    family = MONOMIAL
    order = CONSTANT
  []
  [temp_bins]
    family = MONOMIAL
    order = CONSTANT
  []
  [bins]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [rho]
    type = FunctionAux
    variable = rho
    function = rho
    execute_on = timestep_begin
  []
  [temp]
    type = FunctionAux
    variable = temp
    function = temp
    execute_on = timestep_begin
  []
  [rho_bins]
    type = SkinnedBins
    variable = rho_bins
    skinner = moab
    skin_by = rho
  []
  [temp_bins]
    type = SkinnedBins
    variable = temp_bins
    skinner = moab
    skin_by = temp
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
    expression = '400 + x * 100 + 100 * t'
  []
  [rho]
    type = ParsedFunction
    expression = '400 + y * 100 + 100 * t'
  []
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    fields = 'temp rho'
    fields_min = '295 400'
    fields_max = '715 715'
    n_field_bins = '3 10'
    verbose = true
    material_blocks = '2 3'
    material_names = 'mat2 mat3'



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
