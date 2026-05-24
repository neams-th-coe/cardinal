[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  cell_level = 0

  # Add one tally to ensure time is spent during tally accumulation.
  [Tallies]
    [Cell]
      type = CellTally

      check_tally_sum = false
      normalize_by_global_tally = false
    []
  []
[]

[Executioner]
  type = Transient
  # Run multiple steps to test accumulating walltime over multiple simulations.
  num_steps = 2
[]

[Postprocessors]
  # Return the time spent on the current timestep.
  [step_initialization_time]
    type = OpenMCWallTime
    accumulate_time = false
    time_type = 'initialization_time'
    execute_on = 'TIMESTEP_END'
  []
  [step_total_simulation_time]
    type = OpenMCWallTime
    accumulate_time = false
    time_type = 'total_simulation_time'
    execute_on = 'TIMESTEP_END'
  []
  [step_transport_time]
    type = OpenMCWallTime
    accumulate_time = false
    time_type = 'transport_time'
    execute_on = 'TIMESTEP_END'
  []
  [step_inactive_batch_time]
    type = OpenMCWallTime
    accumulate_time = false
    time_type = 'inactive_batch_time'
    execute_on = 'TIMESTEP_END'
  []
  [step_active_batch_time]
    type = OpenMCWallTime
    accumulate_time = false
    time_type = 'active_batch_time'
    execute_on = 'TIMESTEP_END'
  []
  [step_fission_bank_time]
    type = OpenMCWallTime
    accumulate_time = false
    time_type = 'fission_bank_time'
    execute_on = 'TIMESTEP_END'
  []
  [step_tally_accumulation_time]
    type = OpenMCWallTime
    accumulate_time = false
    time_type = 'tally_accumulation_time'
    execute_on = 'TIMESTEP_END'
  []
  [step_finalization_time]
    type = OpenMCWallTime
    accumulate_time = false
    time_type = 'finalization_time'
    execute_on = 'TIMESTEP_END'
  []
  [step_total_time_elapsed]
    type = OpenMCWallTime
    accumulate_time = false
    time_type = 'total_time_elapsed'
    execute_on = 'TIMESTEP_END'
  []

  # Return the accumulated time.
  [accum_initialization_time]
    type = OpenMCWallTime
    time_type = 'initialization_time'
    execute_on = 'TIMESTEP_END'
  []
  [accum_total_simulation_time]
    type = OpenMCWallTime
    time_type = 'total_simulation_time'
    execute_on = 'TIMESTEP_END'
  []
  [accum_transport_time]
    type = OpenMCWallTime
    time_type = 'transport_time'
    execute_on = 'TIMESTEP_END'
  []
  [accum_inactive_batch_time]
    type = OpenMCWallTime
    time_type = 'inactive_batch_time'
    execute_on = 'TIMESTEP_END'
  []
  [accum_active_batch_time]
    type = OpenMCWallTime
    time_type = 'active_batch_time'
    execute_on = 'TIMESTEP_END'
  []
  [accum_fission_bank_time]
    type = OpenMCWallTime
    time_type = 'fission_bank_time'
    execute_on = 'TIMESTEP_END'
  []
  [accum_tally_accumulation_time]
    type = OpenMCWallTime
    time_type = 'tally_accumulation_time'
    execute_on = 'TIMESTEP_END'
  []
  [accum_finalization_time]
    type = OpenMCWallTime
    time_type = 'finalization_time'
    execute_on = 'TIMESTEP_END'
  []
  [accum_total_time_elapsed]
    type = OpenMCWallTime
    time_type = 'total_time_elapsed'
    execute_on = 'TIMESTEP_END'
  []

  # Testing simulation runtime is often indeterminate due to differences
  # in testing hardware and hardware utilization at the time of testing. These
  # post-processors harden the testing suite against failures by checking to make
  # sure a time is reported. The output has been manually confirmed to match against
  # the console display from OpenMC.
  [step_initialization_time_gzero]
    type = ParsedPostprocessor
    expression = 'step_initialization_time > 1e-8'
    pp_names = 'step_initialization_time'
    execute_on = 'TIMESTEP_END'
  []
  [step_total_simulation_time_gzero]
    type = ParsedPostprocessor
    expression = 'step_total_simulation_time > 1e-8'
    pp_names = 'step_total_simulation_time'
    execute_on = 'TIMESTEP_END'
  []
  [step_transport_time_gzero]
    type = ParsedPostprocessor
    expression = 'step_transport_time > 1e-8'
    pp_names = 'step_transport_time'
    execute_on = 'TIMESTEP_END'
  []
  [step_inactive_batch_time_gzero]
    type = ParsedPostprocessor
    expression = 'step_inactive_batch_time > 1e-8'
    pp_names = 'step_inactive_batch_time'
    execute_on = 'TIMESTEP_END'
  []
  [step_active_batch_time_gzero]
    type = ParsedPostprocessor
    expression = 'step_active_batch_time > 1e-8'
    pp_names = 'step_active_batch_time'
    execute_on = 'TIMESTEP_END'
  []
  [step_fission_bank_time_gzero]
    type = ParsedPostprocessor
    expression = 'step_fission_bank_time > 1e-8'
    pp_names = 'step_fission_bank_time'
    execute_on = 'TIMESTEP_END'
  []
  [step_tally_accumulation_time_gzero]
    type = ParsedPostprocessor
    expression = 'step_tally_accumulation_time > 1e-8'
    pp_names = 'step_tally_accumulation_time'
    execute_on = 'TIMESTEP_END'
  []
  [step_finalization_time_gzero]
    type = ParsedPostprocessor
    expression = 'step_finalization_time > 1e-8'
    pp_names = 'step_finalization_time'
    execute_on = 'TIMESTEP_END'
  []
  [step_total_time_elapsed_gzero]
    type = ParsedPostprocessor
    expression = 'step_total_time_elapsed > 1e-8'
    pp_names = 'step_total_time_elapsed'
    execute_on = 'TIMESTEP_END'
  []

  [accum_initialization_time_gzero]
    type = ParsedPostprocessor
    expression = 'accum_initialization_time > 1e-8'
    pp_names = 'accum_initialization_time'
    execute_on = 'TIMESTEP_END'
  []
  [accum_total_simulation_time_gzero]
    type = ParsedPostprocessor
    expression = 'accum_total_simulation_time > 1e-8'
    pp_names = 'accum_total_simulation_time'
    execute_on = 'TIMESTEP_END'
  []
  [accum_transport_time_gzero]
    type = ParsedPostprocessor
    expression = 'accum_transport_time > 1e-8'
    pp_names = 'accum_transport_time'
    execute_on = 'TIMESTEP_END'
  []
  [accum_inactive_batch_time_gzero]
    type = ParsedPostprocessor
    expression = 'accum_inactive_batch_time > 1e-8'
    pp_names = 'accum_inactive_batch_time'
    execute_on = 'TIMESTEP_END'
  []
  [accum_active_batch_time_gzero]
    type = ParsedPostprocessor
    expression = 'accum_active_batch_time > 1e-8'
    pp_names = 'accum_active_batch_time'
    execute_on = 'TIMESTEP_END'
  []
  [accum_fission_bank_time_gzero]
    type = ParsedPostprocessor
    expression = 'accum_fission_bank_time > 1e-8'
    pp_names = 'accum_fission_bank_time'
    execute_on = 'TIMESTEP_END'
  []
  [accum_tally_accumulation_time_gzero]
    type = ParsedPostprocessor
    expression = 'accum_tally_accumulation_time > 1e-8'
    pp_names = 'accum_tally_accumulation_time'
    execute_on = 'TIMESTEP_END'
  []
  [accum_finalization_time_gzero]
    type = ParsedPostprocessor
    expression = 'accum_finalization_time > 1e-8'
    pp_names = 'accum_finalization_time'
    execute_on = 'TIMESTEP_END'
  []
  [accum_total_time_elapsed_gzero]
    type = ParsedPostprocessor
    expression = 'accum_total_time_elapsed > 1e-8'
    pp_names = 'accum_total_time_elapsed'
    execute_on = 'TIMESTEP_END'
  []
[]

[Outputs]
  execute_on = 'TIMESTEP_END'
  csv = true
  hide = 'step_initialization_time      step_total_simulation_time  step_transport_time
          step_inactive_batch_time      step_active_batch_time      step_fission_bank_time
          step_tally_accumulation_time  step_finalization_time      step_total_time_elapsed
          accum_initialization_time     accum_total_simulation_time accum_transport_time
          accum_inactive_batch_time     accum_active_batch_time     accum_fission_bank_time
          accum_tally_accumulation_time accum_finalization_time     accum_total_time_elapsed'
[]
