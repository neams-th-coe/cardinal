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
    epsilon = 1e-8
  []
  [id2]
    type = ParsedSubdomainMeshGenerator
    input = id1
    combinatorial_geometry = 'z >= 0.0'
    block_id = 3
    epsilon = 1e-8
  []

  second_order = true
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

    density = rho
    n_density_bins = 5
    density_min = 445.0
    density_max = 655.0

    output_skins = true
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  exodus = true
  hide = 'temp rho'
[]
