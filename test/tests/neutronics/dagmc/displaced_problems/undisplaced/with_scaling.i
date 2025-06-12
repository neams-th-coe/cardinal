scale = 100.0

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 8
    ny = 4
    nz = 2
    xmin = -0.125
    ymin = -0.125
    zmin = -0.125
    xmax = 0.875
    ymax = 0.375
    zmax = 0.125
  []
  [block_1]
    type = ParsedSubdomainMeshGenerator
    input = gmg
    combinatorial_geometry = 'y>0.125'
    block_id = 1
  []
  [convert]
    type = ElementsToTetrahedronsConverter
    input = block_1
  []
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

[Functions]
  [temp]
    type = ParsedFunction
    expression = '500.0 + 10.0*x*${scale}'
  []
[]

[AuxKernels]
  [temp]
    type = FunctionAux
    variable = temp
    function = temp
    execute_on = timestep_begin
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  scaling = ${scale}

  temperature_blocks = '0 1'
  cell_level = 0
  power = 16.0

  skinner = moab

  [Tallies]
    [Cell]
      type = CellTally
      block = '0 1'
    []
  []
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    temperature_min = 300.0
    temperature_max = 1500.0
    n_temperature_bins = 10
    temperature = temp
    build_graveyard = true
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
  exodus = true
[]
