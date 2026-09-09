# Restarting Coupled NekRS and MOOSE simulations

In this tutorial, you will learn how to:

- Create checkpoint files for NekRS simulations
- Create checkpoint files for MOOSE simulations
- Use checkpoint files to restart coupled NekRS-MOOSE simulations

To access this tutorial,

```
cd cardinal/tutorials/restart_nek_and_moose
```

This tutorial will describe both how to restart NekRS simulations and how to
restart MOOSE simulations; you do not need to use these features together in
one simulation (e.g. you can restart MOOSE while initializing a fresh NekRS
simulation, or vice versa) - but they can also be combined together (restarting
both MOOSE and NekRS from files within the same simulation).

## Creating checkpoint files for NekRS

NekRS checkpoint file output is controlled
using the `writeControl` and `writeInterval` parameters in the `.par` file.
The `.par` example below, for instance, writes a single checkpoint file
at the end of the simulation (because this case runs one time step, and even
though `writeInterval` is 2, we always write one time step at the last step in NekRS).
More fine-grained control can be achieved by using a smaller `writeInterval`.

!listing /tutorials/restart_nek_and_moose/create_checkpoints/pyramid.par

These output files are generated when running NekRS, which for this example can
be done using the `nrsmpi` executable or via Cardinal wrapping,

```
cd create_checkpoints
cardinal-opt -i nek.i
```

## Creating checkpoint files for MOOSE

MOOSE checkpoint file output is controlled
using [Outputs/Checkpoint](Checkpoint.md).
This is one of the [Restart and Recover](restart_recover.md)
options in the MOOSE framework. This checkpoint method requires N to N restarting, where the number of processors
for the previous and current MOOSE simulations must match.

!listing /tutorials/restart_nek_and_moose/create_checkpoints/main.i
  block=Outputs

To run the MOOSE input file and create our checkpoint file,

```
cd create_checkpoints
cardinal-opt -i main.i
```

## Reading checkpoint files for NekRS

The NekRS solution is restarted using the `startFrom`
parameter in the NekRS `.par` file. `startFrom` should contain the
checkpoint file to restart the NekRS portion of the coupled simulation.
In this folder, there is a symbolic link to the output file we generated
in the previous portion of this tutorial.

!listing /tutorials/restart_nek_and_moose/read_from_checkpoints/pyramid.par

You should then be sure to not set any initial condition on fields you wish
to load from the restart file; for instance, in this new case where we are
using the restart file, the `UDF_Setup` function is empty:

!listing /tutorials/restart_nek_and_moose/read_from_checkpoints/pyramid.udf language=cpp

To run the case,

```
cd read_from_checkpoints
cardinal-opt -i nek.i
```

!alert note
The `start_time` for a NekRS simulation is defined on the `Executioner` block
in the NekRS-wrapped input file. By default, the start time will be set to zero
(though the solution will still be read from whatever data is provided in the
`startFrom` parameter in the `.par` file). In order to properly continue a
coupled MOOSE-NekRS simulation, you may need to set the `start_time` in the
MOOSE input file to match what NekRS will use. For instance, if you keep the
default behavior in NekRS of setting the time to zero, set the `start_time` in
the coupled MOOSE file also to zero.

## Reading checkpoint files for MOOSE

The MOOSE solution is restarted within its Problem block
using `restart_file_base`. This should contain the
checkpoint file base to restart the MOOSE portion of the
coupled simulation from. Furthermore, if MOOSE is running as the sub app in the MultiApp
system, `force_restart` can be set to restart the MOOSE sub app
without restarting the main app.

!listing /tutorials/restart_nek_and_moose/read_from_checkpoints/main.i
  block=Problem

!listing /tutorials/restart_nek_and_moose/read_from_checkpoints/main.i
  block=Executioner

To run the MOOSE simulation with restarted fields,

```
cd read_from_checkpoints
cardinal-opt -i main.i
```
