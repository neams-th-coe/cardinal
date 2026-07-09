[Mesh]
  [f]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -12.5
    xmax = 87.5
    ymin = -12.5
    ymax = 37.5
    zmin = -12.5
    zmax = 12.5
    nx = 8
    ny = 4
    nz = 2
  []
  [subdomains]
    type = ParsedSubdomainIDsGenerator
    input = f
    expression = 'if (y > 12.5, 1, 2)'
  []
  [name]
    type = RenameBlockGenerator
    input = subdomains
    old_block = '1 2'
    new_block = 'block_1 block_2'
  []
  # The following sidesets/nodesets are only used by the error-checking and internal-surface tests
  [left_dup] # duplicate of the 'left' sideset, to test one side belonging to two sidesets
    type = ParsedGenerateSideset
    input = name
    combinatorial_geometry = 'x < -12.49'
    normal = '-1 0 0'
    new_sideset_name = 'left_dup'
  []
  [interface_a] # the block_1/block_2 interface, attached to block_1 element sides
    type = SideSetsBetweenSubdomainsGenerator
    input = left_dup
    primary_block = 'block_1'
    paired_block = 'block_2'
    new_boundary = 'interface_a'
  []
  [interface_b] # the same interface, attached to block_2 element sides
    type = SideSetsBetweenSubdomainsGenerator
    input = interface_a
    primary_block = 'block_2'
    paired_block = 'block_1'
    new_boundary = 'interface_b'
  []
  [midplane] # a plane interior to block_2 (y = 0), which never appears in any skin
    type = ParsedGenerateSideset
    input = interface_b
    combinatorial_geometry = 'y > -0.01 & y < 0.01'
    normal = '0 1 0'
    new_sideset_name = 'midplane'
  []
  [corner_nodes] # a nodeset-only boundary, to test the nodeset error message
    type = ExtraNodesetGenerator
    input = midplane
    new_boundary = 'corner_nodes'
    coord = '-12.5 -12.5 -12.5'
  []

  allow_renumbering = false
  parallel_type = replicated
[]

[AuxVariables]
  [cell_temperature]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_temperature]
    type = CellTemperatureAux
    variable = cell_temperature
  []
[]

[AuxKernels]
  [temp]
    type = ConstantAux
    variable = temp
    value = 500.0
    execute_on = timestep_begin
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  temperature_blocks = '1 2'
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
    # BCs are added in tests as cli_args to use this file in all different cases
    type = MoabSkinner
    verbose = true
    temperature_min = 0.0
    temperature_max = 1000.0
    n_temperature_bins = 1
    temperature = temp
  []
[]

[Postprocessors]
  [k]
    type = KEigenvalue
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = false
  [file]
    type = CSV
  []
[]
