# Restarting Coupled NekRS and MOOSE simulations

This tutorial describes how to restart coupled NekRS
and MOOSE simulations. First, the user needs to output
restartable solution/checkpoint files for both NekRS and MOOSE.
Then, the user needs to restart both simulations
using said checkpoint files.

# Creating NekRS checkpoint files

NekRS checkpoint file output is controlled
using the `writeControl` parameter in the .par file.
The .par example below only writes a single checkpoint file
at the end of the simulation.

!listing /tutorials/restart_nek_and_moose/create_checkpoints/pyramid.par

# Creating MOOSE checkpoint files

MOOSE checkpoint file output is controlled
using [Outputs/Checkpoint](https://mooseframework.inl.gov/source/outputs/Checkpoint.html).
This is one of the [Restart and Recover](https://mooseframework.inl.gov/application_usage/restart_recover.html)
options in the MOOSE framework. This checkpoint method requires N to N restarting, where the number of processors
for the previous and current MOOSE simulations must match.

!listing /tutorials/restart_nek_and_moose/create_checkpoints/cardinal_sub.i
  block=Outputs

# Reading NekRS checkpoint files

The NekRS solution is restarted using the `restartFrom`
parameter in the NekRS .par file. `restartFrom` should contain the
checkpoint file to restart the NekRS portion of the coupled simulation from.

!listing /tutorials/restart_nek_and_moose/read_from_checkpoints/pyramid.par

# Reading MOOSE checkpoint files

The MOOSE solution is restarted within its Problem block
using `restart_file_base`. `restart_file_base` should contain the
checkpoint file base to restart the MOOSE portion of the
coupled simulation from. Furthermore, if MOOSE is running as the sub app in the MultiApp
system, `force_restart` can be set to restart the MOOSE sub app
without restarting the main app.

!listing /tutorials/restart_nek_and_moose/read_from_checkpoints/cardinal_sub.i
  block=Problem

# Restart MOOSE at time zero

By default, NekRS will restart its simulation from time zero. The `start_time` in MOOSE
needs to be set to zero in order for MOOSE to start at the same time as the NekRS simulation.

!listing /tutorials/restart_nek_and_moose/read_from_checkpoints/cardinal_sub.i
  block=Executioner
