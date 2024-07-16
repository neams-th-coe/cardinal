scale = 100.0

[Mesh]
  [file]
    type = FileMeshGenerator
    file = scale_in.e
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

[Tallies]
  [Mesh]
    type = MeshTally
    mesh_template = scale_in.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  scaling = ${scale}

  temperature_blocks = '1 2'
  cell_level = 0
  power = 16.0

  skinner = moab
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
