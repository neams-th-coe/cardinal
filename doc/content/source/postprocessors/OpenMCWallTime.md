# OpenMCWallTime

!syntax description /Postprocessors/OpenMCWallTime

## Description

This post-processor outputs various timing statistics from OpenMC, the particular timing statistic can be
select with [!param](/Postprocessors/OpenMCWallTime/time_type). Options are:

- `initialization_time`: the time spent initializing the OpenMC simulation
- `total_simulation_time`: the total time spent running the OpenMC simulation
- `transport_time`: the total time spent in transport (sum of time spent in active and inactive batches)
- `inactive_batch_time`: the total time spent running inactive batches
- `active_batch_time`: the total time spent running active batches
- `fission_bank_time`: the total time spent synchronizing the fission bank
- `tally_accumulation_time`: the total time spent accumulating tally statistics across MPI ranks
- `finalization_time`: the total time spent finalizing the OpenMC simulation
- `total_time_elapsed`: the total time spent executing OpenMC

Either the cumulative simulation time (over all time steps / Picard iterations / adaptivity steps) or the per-step
walltime may be selected by setting [!param](/Postprocessors/OpenMCWallTime/accumulate_time).

## Example Input Syntax

An example of using this post-processor for outputting timing statistics can be found below:

!listing test/tests/postprocessors/openmc_walltime/openmc.i
  block=Postprocessors

!syntax parameters /Postprocessors/OpenMCWallTime

!syntax inputs /Postprocessors/OpenMCWallTime
