[Mesh]
 [cube]
    type = FileMeshGenerator
    file = '../../../neutronics/meshes/tet_cube.e'
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

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  use_displaced_mesh = true
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
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
    expression = 1000+300*x*x+100*t
  []
  [rho]
    type = ParsedFunction
    expression = 1000+300*y*y+100*t
  []
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    verbose = true
    material_names = "mat mat"

    temperature = temp
    n_temperature_bins = 10
    temperature_min = 996.0
    temperature_max = 1070.0

    density = rho
    n_density_bins = 10
    density_min = 996.0
    density_max = 1070.0

    output_skins = true
  []
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
  hide = 'temp rho'
[]
