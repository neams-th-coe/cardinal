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

    # will work okay if the input file is run with at least 2 MPI ranks (one rank per Nek solve)
    num_rows = 3

    distributions = 'uniform'
    execute_on = INITIAL # create random numbers on initial and use them for each timestep
  []
[]

[MultiApps]
  [nek]
    type = SamplerFullSolveMultiApp
    input_files = nek_multi.i
    sampler = sample
    mode = normal
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

  # The time step size here has no effect for SamplerFullSolveMultiApp. If we use > 1
  # time step, it will try to run the same full nek.i simulation that many times.
[]

[Outputs]
  execute_on = 'INITIAL TIMESTEP_END'
[]
