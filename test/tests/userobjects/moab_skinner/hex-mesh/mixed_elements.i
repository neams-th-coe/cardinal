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
    skin_by = density
  []
  [temp_bins]
    type = SkinnedBins
    variable = temp_bins
    skinner = moab
    skin_by = temperature
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
    verbose = true
    material_names = 'mat2 mat3'

    temperature = temp
    n_temperature_bins = 3
    temperature_min = 295.0
    temperature_max = 715.0

    density = rho
    n_density_bins = 10
    density_min = 400.0
    density_max = 715.0

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
