# Restarting Coupled NekRS and MOOSE simulations

In this tutorial, you will learn how to:

- Create checkpoint files for NekRS simulations
- Create checkpoint files for MOOSE simulations
- Use checkpoint files to restart coupled NekRS-MOOSE simulations

To access this tutorial,

```
cd cardinal/tutorials/restart_nek_and_moose
```

## Creating checkpoint files

NekRS checkpoint file output is controlled
using the `writeControl` and `writeInterval` parameters in the `.par` file.
The `.par` example below, for instance, writes a single checkpoint file
at the end of the simulation. More fine-grained control can be achieved
by using a smaller `writeInterval`.

!listing /tutorials/restart_nek_and_moose/create_checkpoints/pyramid.par

MOOSE checkpoint file output is controlled
using [Outputs/Checkpoint](https://mooseframework.inl.gov/source/outputs/Checkpoint.html).
This is one of the [Restart and Recover](https://mooseframework.inl.gov/application_usage/restart_recover.html)
options in the MOOSE framework. This checkpoint method requires N to N restarting, where the number of processors
for the previous and current MOOSE simulations must match.

!listing /tutorials/restart_nek_and_moose/create_checkpoints/main.i
  block=Outputs

To run these input files and create our checkpoint files,

```
cd create_checkpoints
cardinal-opt -i main.i
```

## Reading checkpoint files

The NekRS solution is restarted using the `startFrom`
parameter in the NekRS `.par` file. `startFrom` should contain the
checkpoint file to restart the NekRS portion of the coupled simulation.

!listing /tutorials/restart_nek_and_moose/read_from_checkpoints/pyramid.par

The MOOSE solution is restarted within its Problem block
using `restart_file_base`. This should contain the
checkpoint file base to restart the MOOSE portion of the
coupled simulation from. Furthermore, if MOOSE is running as the sub app in the MultiApp
system, `force_restart` can be set to restart the MOOSE sub app
without restarting the main app.

!listing /tutorials/restart_nek_and_moose/read_from_checkpoints/main.i
  block=Problem

Finally, the `start_time` for a NekRS simulation is defined on the `Executioner` block
in the NekRS-wrapped input file. By default, the start time will be set to zero
(though the solution will still be read from whatever data is provided in the
`startFrom` parameter in the `.par` file). In order to properly continue a
coupled MOOSE-NekRS simulation, you just need to set the `start_time` in the
MOOSE input file to match what NekRS will use. For instance, if you keep the
default behavior in NekRS of setting the time to zero, set the `start_time` in
the coupled MOOSE file also to zero.

!listing /tutorials/restart_nek_and_moose/read_from_checkpoints/main.i
  block=Executioner

To run the simulation with restarted fields,

```
cd read_from_checkpoints
cardinal-opt -i main.i
```
