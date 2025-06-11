[Mesh]
  [slab]
    type = FileMeshGenerator
    file = ../../mesh_tallies/slab.e
  []
  [cube]
    type = FileMeshGenerator
    file = ../../../meshes/tet_cube.e
  []
  [rename_cube]
    type = RenameBlockGenerator
    input = cube
    old_block = '1'
    new_block = '3'
  []
  [combine]
    type = CombinerGenerator
    inputs = 'slab rename_cube'
    positions = '0 0 0 0 50 0'
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
  temperature_blocks = '1 2 3'
  cell_level = 0
  power = 100.0

  skinner = moab


  [Tallies]
    [Cell]
      type = CellTally
      block = '1 2 3'
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
  exodus = true
  hide = 'temp'
[]
