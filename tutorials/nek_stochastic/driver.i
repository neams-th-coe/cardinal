[StochasticTools]
[]

[Distributions]
  [dist]
    type = Normal
    mean = 5.0
    standard_deviation = 1.0
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 3
    distributions = 'dist'
  []
[]

[MultiApps]
  [ht]
    type = SamplerFullSolveMultiApp
    input_files = ht.i
    sampler = sample
    mode = batch-restore
    wait_for_first_app_init = true
  []
[]

[Transfers]
  [transer_random_inputs_to_nek]
    type = SamplerParameterTransfer
    to_multi_app = ht
    sampler = sample

    # you can send data down to arbitrarily-nested sub-apps
    parameters = 'nek:Problem/ScalarTransfers/k/value'
  []
  [results]
    type = SamplerReporterTransfer
    from_multi_app = ht
    sampler = sample
    stochastic_reporter = storage
    from_reporter = 'receive/nek_max_T'
  []
[]

[Reporters]
  [storage]
    type = StochasticMatrix
    sampler = sample
    parallel_type = ROOT
  []
  [stats]
    type = StatisticsReporter
    reporters = 'storage/results:receive:nek_max_T'
    compute = 'mean stddev'
    ci_method = 'percentile'
    ci_levels = '0.05 0.95'
  []
[]

[Outputs]
  execute_on = timestep_end
  [out]
    type = JSON
  []
[]
