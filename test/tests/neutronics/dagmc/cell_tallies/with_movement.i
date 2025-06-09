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

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  use_displaced_mesh = true
[]

[Variables]
  [disp_x]
    initial_condition = 0.0
  []
  [disp_y]
    initial_condition = 0.0
  []
  [disp_z]
    initial_condition = 0.0
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
  fixed_mesh = false

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
  num_steps = 2
[]

[MultiApps]
  [move]
    type = TransientMultiApp
    input_files = 'move.i'
    execute_on = timestep_end
  []
[]

[Transfers]
  [dispx]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    source_variable = disp_x
    variable = disp_x
    from_multi_app = move
  []
  [dispy]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    source_variable = disp_y
    variable = disp_y
    from_multi_app = move
  []
  [dispz]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    source_variable = disp_z
    variable = disp_z
    from_multi_app = move
  []
[]

[Outputs]
  exodus = true
[]
