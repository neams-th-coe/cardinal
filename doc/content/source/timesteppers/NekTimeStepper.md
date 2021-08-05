# NekTimeStepper

!syntax description /Executioner/TimeStepper/NekTimeStepper

## Description

This class allows the wrapping of NekRS in [NekRSProblem](/problems/NekRSProblem.md)
to control its own time stepping within a coupled solution. In other words, whatever time
stepping is specified in the `.par` file
(and any adaptive time stepping schemes in NekRS) is used to control the time stepping
when wrapped as a MOOSE application.

There are two different possibilities for how this time stepper controls
the time step used in a MultiApp simulation using NekRS. If NekRS is the *master* application,
then the time step used by NekRS is entirely dictated by NekRS's internal time
step update routines. In addition, the `endTime` and `numSteps` settings in the `.par` file
are respected - the simulation will terminate based on the conditions in the `.par` file.

If NekRS is instead a *sub* application, we allow the master application controlling
NekRS to dictate the time stepping. This is necessary to enable sub-cycling, app synchronizations,
and terminations based on coupled physics criteria that NekRS as a standalone tool
is unaware of. So, when NekRS is a sub-application, the time step size desired by NekRS
is respected to the maximum extent possible. For instance, if NekRS wants to take a time step of 0.3 seconds,
but a master application takes a time step of 1.0 seconds, then for each 1-second master
application step, NekRS as a sub-application will perform:

- Three time steps equal to 0.3 seconds
- One time step equal to 0.1 seconds

Allowing the master application to control NekRS's time stepping in this manner
is necessary to allow applications to synchronize. When NekRS is run
as a sub-application, the `endTime` and `numSteps` settings in `.par` file are
ignored. Instead, the simulation termination critera of the master application
dictate the overall solve.

If running NekRS in non-dimensional form (and you have indicated the
appropriate nondimensional scales by setting `nondimensional = true`
for [NekRSProblem](/problems/NekRSProblem.md)), then the time is converted to dimensional
units when used in the [MultiApp](https://mooseframework.inl.gov/syntax/MultiApps/index.html) system,
under the assumption that all other coupled MOOSE applications use a dimensional
time unit. Finally, the minimum time step size that can be taken in NekRS is controlled via
the `min_dt` parameter.

## Example Input Syntax

!listing /test/tests/cht/pebble/nek.i
  block=Executioner

!syntax parameters /Executioner/TimeStepper/NekTimeStepper

!syntax inputs /Executioner/TimeStepper/NekTimeStepper

!syntax children /Executioner/TimeStepper/NekTimeStepper
