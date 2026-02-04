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
  [hextotet]
    type = ElementsToTetrahedronsConverter
    input = name
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
  cell_level = 1
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
    verbose = true
    temperature_min = 0.0
    temperature_max = 1000.0
    n_temperature_bins = 1
    temperature = temp
    build_graveyard = false
  []
[]

[Postprocessors]
  [k]
    type = KEigenvalue
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
