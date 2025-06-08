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
    num_rows = 3
    distributions = 'uniform'
  []
[]

[MultiApps]
  [nek]
    type = SamplerFullSolveMultiApp
    input_files = nek_multi.i
    sampler = sample
    mode = batch-restore
    wait_for_first_app_init = true
  []
[]

[Transfers]
  [transer_random_inputs_to_nek]
    type = SamplerParameterTransfer
    to_multi_app = nek
    sampler = sample
    parameters = 'Problem/ScalarTransfers/scalar1/value'
  []
  [results]
    type = SamplerReporterTransfer
    from_multi_app = nek
    sampler = sample
    stochastic_reporter = results
    from_reporter = 'max_scalar/value'
  []
[]

[Reporters]
  [results]
    type = StochasticReporter
  []
  [stats]
    type = StatisticsReporter
    reporters = 'results/results:max_scalar:value'
    compute = 'mean stddev'
    ci_method = 'percentile'
    ci_levels = '0.05 0.95'
  []
[]

[Outputs]
  execute_on = final

  [reporter]
    type = JSON
  []
[]
