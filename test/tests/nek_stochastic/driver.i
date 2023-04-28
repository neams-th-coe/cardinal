[StochasticTools]
  auto_create_executioner = false
[]

[Distributions]
  [uniform]
    type = Uniform
    lower_bound = 0
    upper_bound = 1
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 1
    distributions = 'uniform'
    execute_on = INITIAL # create random numbers on initial and use them for each timestep
    min_procs_per_row = 1
  []
[]

[MultiApps]
  [nek]
    type = SamplerFullSolveMultiApp
    input_files = nek.i
    sampler = sample
    mode = normal
    min_procs_per_app = 1
  []
[]

[Transfers]
  [nek]
    type = SamplerParameterTransfer
    to_multi_app = nek
    sampler = sample
    parameters = 'UserObjects/scalar1/value'
    check_multiapp_execute_on = false
  []
[]

[Executioner]
  type = Transient
  num_steps = 1

  # the time step size here has no effect for SamplerFullSolveMultiApp
[]

[Outputs]
  execute_on = 'INITIAL TIMESTEP_END'
[]
