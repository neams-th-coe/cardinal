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

x0 = 12.5
x1 = 37.5
x2 = 62.5

T0 = 600.0
dT = 50.0

[Functions]
  [temp]
    type = ParsedFunction
    expression = 'if (x <= ${x0}, ${fparse T0 - dT}, if (x <= ${x1}, ${T0}, if (x <= ${x2}, ${fparse T0 + dT}, ${fparse T0 + 2 * dT})))'
  []
[]

[AuxVariables]
  [material_id]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [temp]
    type = FunctionAux
    variable = temp
    function = temp
    execute_on = timestep_begin
  []
  [density]
    type = FunctionAux
    variable = density
    function = temp
    execute_on = timestep_begin
  []
  [material_id]
    type = CellMaterialIDAux
    variable = material_id
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  temperature_blocks = '1 2'
  density_blocks = '1 2'
  cell_level = 0
  power = 100.0

  skinner = moab

  [Tallies]
    [Cell]
      type = CellTally
      block = '1 2'
    []
  []
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    temperature_min = 0
    temperature_max = 1000
    density_min = 0
    density_max = 1000
    n_temperature_bins = 4
    n_density_bins = 1
    temperature = temp
    density = density
    build_graveyard = true
    graveyard_scale_inner = 1.1
    graveyard_scale_outer = 1.2
    material_blocks = 'block_1 2'
    material_names = 'h2o 1'
  []
[]

[Postprocessors]
  [mat_in_block_1]
    type = PointValue
    variable = material_id
    point = '12.5 12.5 12.5'
  []
  [mat_in_block_2]
    type = PointValue
    variable = material_id
    point = '62.5 12.5 12.5'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
