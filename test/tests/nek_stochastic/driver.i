[StochasticTools]
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

[Outputs]
  execute_on = final
[]
