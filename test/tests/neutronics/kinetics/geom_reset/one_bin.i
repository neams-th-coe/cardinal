[Mesh]
  type = FileMesh
  file = ../../dagmc/mesh_tallies/slab.e
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

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  temperature_blocks = '1 2'
  cell_level = 0
  power = 100.0

  skinner = moab

  calc_kinetics_params = true
  ifp_generations = 5

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
  [lambda]
    type = LambdaEffective
  []
  [lambda_rel]
    type = LambdaEffective
    output = rel_err
  []
  [lambda_std]
    type = LambdaEffective
    output = std_dev
  []
  [beta]
    type = BetaEffective
  []
  [beta_rel]
    type = BetaEffective
    output = rel_err
  []
  [beta_std]
    type = BetaEffective
    output = std_dev
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
