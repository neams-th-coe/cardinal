[Mesh]
  [file]
    type = FileMeshGenerator
    file = ../mesh_tallies/slab.e
  []
  allow_renumbering = false
[]

[Functions]
  [temp]
    type = ParsedFunction
    expression = '500.0 + 10.0*x'
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

  temperature_blocks = '1 2'
  cell_level = 0
  power = 16.0

  skinner = moab

  relaxation = 'constant'

  [Tallies]
    [Mesh]
      type = CellTally
      block = '1 2'
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

[Executioner]
  type = Transient
  num_steps = 1
[]
