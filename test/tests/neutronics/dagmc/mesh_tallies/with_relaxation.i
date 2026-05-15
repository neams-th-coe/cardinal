[Mesh]
  [file]
    type = FileMeshGenerator
    file = slab.e
  []
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
  verbose = true

  temperature_blocks = '1 2'
  cell_level = 0
  power = 16.0

  skinner = moab

  relaxation = 'constant'

  [Tallies]
    [Mesh]
      type = MeshTally
      mesh_template = slab.e
      output = 'unrelaxed_tally'
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
  [heating]
    type = PointValue
    variable = kappa_fission
    point = '47 15 5'
  []
  [heating_raw]
    type = PointValue
    variable = kappa_fission_raw
    point = '47 15 5'
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
