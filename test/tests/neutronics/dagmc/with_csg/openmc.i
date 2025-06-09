[Mesh]
  [slab]
    type = FileMeshGenerator
    file = ../mesh_tallies/slab.e
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

  # Since we're ignoring the CSG geometry when mapping tallies, we need to
  # disable the tally sum check.
  check_tally_sum = false

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
    temperature_min = 0.0
    temperature_max = 1000.0
    n_temperature_bins = 1
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

[Outputs]
  csv = true
[]
