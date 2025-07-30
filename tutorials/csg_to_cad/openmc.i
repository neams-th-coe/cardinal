power = 3000

[Problem]
  type = OpenMCCellAverageProblem
  power = ${power}

  temperature_blocks = '1'
  cell_level = 1

  skinner = skinner

  normalize_by_global_tally = false
  source_rate_normalization = 'kappa_fission'

  [Tallies]
    [mesh]
      type = MeshTally
      score = 'kappa_fission flux'
    []
  []
[]

[Mesh]
  [fuel]
    type = FileMeshGenerator
    file = krusty_fuel.e
  []
[]

[UserObjects]
  [skinner]
    type = MoabSkinner
    temperature = temp
    n_temperature_bins = 4
    temperature_min = 800
    temperature_max = 1000
    build_graveyard = true
  []
[]

[MultiApps]
  [conduction]
    type = TransientMultiApp
    input_files = 'fuel.i'
    execute_on = timestep_end
  []
[]

[Transfers]
  [heat_source_from_openmc]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    to_multi_app = conduction
    source_variable = kappa_fission
    variable = power
    from_postprocessors_to_be_preserved = tally_integral
    to_postprocessors_to_be_preserved = tally_integral
  []
  [temp_to_openmc]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    from_multi_app = conduction
    variable = temp
    source_variable = T
  []
[]

[ICs]
  [temp]
    type = ConstantIC
    variable = temp
    value = 800
  []
  [power]
    type = ConstantIC
    variable = kappa_fission
    value = ${fparse power/1.865e+03}
  []
[]

[Postprocessors]
  [tally_integral]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
    execute_on = 'initial transfer timestep_end'
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Outputs]
  exodus = true
[]
