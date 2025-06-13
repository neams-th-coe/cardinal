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
    execute_on = timestep_begin
  []
[]

[MultiApps]
  [nek]
    type = SamplerTransientMultiApp
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
[]

# Using a Transient executioner allows us to send new stochastic values to the sub-apps
# on the synchronization points.
[Executioner]
  type = Transient
  dt = 5e-4
  num_steps = 2
[]

[Outputs]
  execute_on = final
[]
