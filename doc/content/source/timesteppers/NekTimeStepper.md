# NekTimeStepper

!syntax description /Executioner/TimeStepper/NekTimeStepper

## Description

This class allows the wrapping of nekRS in [NekRSProblem](/problems/NekRSProblem.md)
to control its own time stepping within a coupled solution. In other words, whatever time
stepping is specified in the `.par` file is used to control the time stepping
when wrapped as a MOOSE application.

If running nekRS in non-dimensional form (and you have indicated the
appropriate nondimensional scales by setting `nondimensional = true`
for [NekRSProblem](/problems/NekRSProblem.md)), then the time is converted to dimensional
units when used in the [MultiApp](https://mooseframework.inl.gov/syntax/MultiApps/index.html) system,
under the assumption that all other coupled MOOSE applications use a dimensional
time unit.

There are two different possibilities for how this time stepper controls
the time step used in a MultiApp simulation using nekRS. If nekRS is the *master* application,
then the time step used by nekRS is entirely dictated by nekRS's internal time
step update routines. In addition, the `endTime` and `numSteps` settings in the `.par` file
are respected - the simulation will terminate based on the conditions in the `.par` file.

If nekRS is instead a *sub* application, we allow the master application controlling
nekRS to dictate the time stepping. This is necessary to enable sub-cycling, app synchronizations,
and terminations based on coupled physics criteria that nekRS as a standalone tool
is unaware of. So, when nekRS is a sub-application, the time step size desired by nekRS
is respected to the maximum extent possible. For instance, if nekRS wants to take a time step of 0.3 seconds,
but a master application takes a time step of 1.0 seconds, then for each 1-second master
application step, nekRS as a sub-application will perform:

- Three time steps equal to 0.3 seconds
- One time step equal to 0.1 seconds

Allowing the master application to control nekRS's time stepping in this manner
is necessary to allow applications to synchronize. Further, when nekRS is run
as a sub-application, the `endTime` and `numSteps` settings in `.par` file are
ignored. Instead, the simulation termination critera of the master application
dictate the overall solve.

## Example Input Syntax

Below is a code snippet showing common usage of `NekTimeStepper`.

!listing /test/tests/cht/pebble/nek.i
  block=Executioner

!syntax parameters /Executioner/TimeStepper/NekTimeStepper

!syntax inputs /Executioner/TimeStepper/NekTimeStepper

!syntax children /Executioner/TimeStepper/NekTimeStepper
