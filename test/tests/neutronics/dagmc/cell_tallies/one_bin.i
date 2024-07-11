[Mesh]
  type = FileMesh
  file = ../mesh_tallies/slab.e
  allow_renumbering = false
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

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '1 2'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  temperature_blocks = '1 2'
  cell_level = 0
  power = 100.0

  skinner = moab
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
  exodus = true
  hide = 'temp cell_instance'
[]
