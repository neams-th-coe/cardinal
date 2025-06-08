# NekRSProblem

This class performs all activities related to solving NekRS as a MOOSE application.
This class also facilitates data transfers between NekRS's internal solution fields
and MOOSE by reading and writing [MooseVariables](MooseVariable.md),
[Postprocessors](Postprocessor.md), and scalar numbers. You can use this class
to couple NekRS to MOOSE for:

- Boundary [!ac](CHT), by passing heat fluxes and wall temperatures
- Volume coupling via temperature and heat source feedback (such as for coupling to neutronics)
- Fluid-structure interaction, by passing wall displacements and stresses
- Systems-level coupling, by reading and writing scalar numbers representing boundary conditions
- Extracting the NekRS solution for postprocessing or one-way coupling in MOOSE, such as to
  - Query the solution, evaluate heat balances and pressure drops,
    or evaluate solution convergence
  - Providing one-way coupling to other MOOSE applications, such as for
    transporting scalars based on NekRS's velocity solution or for projecting
    NekRS turbulent viscosity closure terms onto another MOOSE application's mesh
  - Project the NekRS solution onto other discretization schemes,
    such as a subchannel discretization, or onto other MOOSE applications, such as
    for providing closures
  - Automatically convert nondimensional NekRS solutions into dimensional form
  - Because the MOOSE framework supports many different [output formats](Outputs/index.md), obtain a representation of the NekRS solution in Exodus, VTK, CSV, and other formats.

All of the above options can be combined together in a flexible, modular system.

!alert! note
This class must be used in conjunction with two other classes in Cardinal:

1. [NekRSMesh](NekRSMesh.md), which builds a mirror of the NekRS mesh
   in a MOOSE format so that all the usual
   [Transfers](Transfers/index.md)
   understand how to send data into/out of NekRS. The settings on
   [NekRSMesh](NekRSMesh.md) also determine which
   coupling types (of those listed above) are available.
2. [NekTimeStepper](NekTimeStepper.md), which allows NekRS to
   control its own time stepping.

!alert-end!

Therefore, we recommend first reading the documentation for the above classes
before proceeding here.

The smallest possible MOOSE-wrapped input file that can be used to run NekRS
is shown below. `casename` is the prefix describing the NekRS input files,
i.e. this parameter would be `casename = 'fluid'` if the NekRS input files
are `fluid.re2`, `fluid.par`, `fluid.udf`, and `fluid.oudf`.

!listing smallest_input.i
  id=smallest
  caption=Smallest possible  NekRS wrapped input file.

The remainder of this page describes how `NekRSProblem` wraps NekRS as a MOOSE application.

## Overall Calculation Methodology

`NekRSProblem` inherits from the [ExternalProblem](ExternalProblem.md)
class. For each time step, the calculation proceeds according to the `ExternalProblem::solve()` function.
Data gets sent into NekRS, NekRS runs a time step, and data gets extracted from NekRS.
`NekRSProblem` mostly consists of defining the `syncSolutions` and `externalSolve` methods.
Each of these functions is now described.

!listing /framework/src/problems/ExternalProblem.C
  re=void\sExternalProblem::solve.*?^}

### External Solve
  id=solve

The actual solve of a timestep by NekRS is peformed within the
`externalSolve` method, which essentially copies NekRS's `main()` function
into MOOSE.

!listing language=cpp
nekrs::runStep(time, dt, time_step_index);
nekrs::ocopyToNek(time + dt, time_step_index);
nekrs::udfExecuteStep(time + dt, time_step_index, is_output_step);
if (is_output_step) nekrs::outfld(time + dt);

These four functions are defined in the NekRS source code, and perform the following:

- Run a single time step
- Copy the device-side solution to the host (so that it can be accessed by MOOSE via AuxVariables, Postprocessors, UserObjects, etc.)
- Execute a [user-defined function](https://nekrsdoc.readthedocs.io/en/latest/input_files.html#udf-executestep-nrs-t-nrs-dfloat-time-int-tstep) in NekRS, `UDF_ExecuteStep`, for Nek-style postprocessing (optional)
- Write a NekRS output file

Because `externalSolve` is wrapped inside two `syncSolutions` calls,
this means that for *every* NekRS time step, data is sent to and from
NekRS, even if NekRS runs with a smaller time step than the MOOSE application
to which it is coupled (i.e. if the data going *into* NekRS hasn't changed since
the last time it was sent to NekRS). A means by which to reduce some of these
(technically) unnecessary data transfers is described in [#min].

### Transfers to NekRS

In the `TO_EXTERNAL_APP` data transfer, [FieldTransfers](AddFieldTransferAction.md)
and [ScalarTransfers](AddScalarTransferAction.md) read from auxvariables and
postprocessors and write data into the NekRS internal data space. Please
click on the links to learn more.

- [FieldTransfers](AddFieldTransferAction.md): passes field data (values defined throughout the nodal points on a mesh) between NekRS and MOOSE
- [ScalarTransfers](AddScalarTransferAction.md): passes scalar data (single values or postprocessors) between NekRS and MOOSE

### Transfer from NekRS

In the `FROM_EXTERNAL_APP` data transfer, [FieldTransfers](AddFieldTransferAction.md)
and [ScalarTransfers](AddScalarTransferAction.md) read from NekRS's internal data
space into auxvariables and postprocessors. Please
click on the links to learn more.

- [FieldTransfers](AddFieldTransferAction.md): passes field data (values defined throughout the nodal points on a mesh) between NekRS and MOOSE
- [ScalarTransfers](AddScalarTransferAction.md): passes scalar data (single values or postprocessors) between NekRS and MOOSE

## Nondimensional Solution

!include nondimensional_problem.md

## Outputting the Scratch Array

This class allows you to write slots in the `nrs->usrwrk` scratch space array
to NekRS field files. This can be useful for viewing the data sent from MOOSE
to NekRS on the actual spectral mesh where they are ultimately used. This
feature can also be used to write field files for other quantities in the scratch
space, such as a wall distance computation
from the Nek5000 backend. To write the scratch space to a field file,
set `usrwrk_output` to an array with each "slot" in the `nrs->usrwrk` array that
you want to write. Then, specify a filename prefix to use to name each field file.

In the example below, the first two "slots" in the `nrs->usrwrk` array will be
written to field files on the same interval that NekRS writes its usual field files.
These files will be named `aaabrick0.f00001`, etc. and `cccbrick0.f00001`, etc.
Based on limitations in how NekRS writes its files, the fields written to these
files will all be named `temperature` when visualized.

!listing test/tests/nek_file_output/usrwrk/nek.i
  block=Problem

## Reducing CPU/GPU Data Transfers
  id=min

As shown in [#solve], for *every* NekRS time step, data is passed in/out of NekRS.
If NekRS is run as a sub-application to a master application, and sub-cycling is used,
a lot of these [!ac](CPU)/[!ac](GPU) data transfers can be omitted.

First, let's explain what MOOSE does in the usual master/sub coupling scheme when
`subcycling = true` using a [!ac](CHT) as an example.
Suppose you have a master application with a time step size of 1 second, and run NekRS
as a sub-application with a time step size of 0.4 seconds that executes at the end of
the master application time step. The calculation procedure involves:

1. Solve the master application from $t$ to $t+1$ seconds.
1. Transfer an auxvariable representing flux (and a postprocessor representing its integral) from the master
  application to the NekRS sub-application) at $t$.
1. Read from the auxvariable and write into the `nrs->usrwrk` array using a [NekBoundaryFlux](NekBoundaryFlux.md) object. Normalize the flux and then copy it from host to device.
1. Run a NekRS time step from $t$ to $t+0.4$ seconds.
1. Copy the temperature from device to host and then interpolate NekRS's
  temperature from NekRS's [!ac](GLL) points to the `NekRSMesh` using a [NekFieldVariable](NekFieldVariable.md).
1. *Even though the flux data hasn't changed*, and
  *even though the temperature data isn't going to be used by the master application yet*,
  `ExternalProblem::solve()`
  performs data transfers in/out of NekRS for *every* NekRS time step. Regardless of
  whether that data has changed or is needed yet by MOOSE, repeat steps 3-5 two times -
  once for a time step size of 0.4 seconds, and again for a time step size of 0.2 seconds
  (for the NekRS sub-application to "catch up" to the master application's overall time
  step length of 1 second.

If NekRS is run with a time step $N$ times smaller than its master
application, this structuring of [ExternalProblem](ExternalProblem.md)
 represents $N-1$ unnecessary
interpolations and [!ac](CPU) to [!ac](GPU) copies of the flux, and $N-1$
unnecessary [!ac](GPU) to [!ac](CPU) copies of the temperature and interpolations.
`NekRSProblem` contains features that allow you to turn off these extra transfers.
However, MOOSE's [MultiApp](MultiApps/index.md)
system is designed in such a way that sub-applications know very little about
their master applications (and for good reason - such a design is what enables such
flexible multiphysics couplings). So, the only way that NekRS can definitively
know that a data transfer from a master application is the *first* data transfer
after the flux data has been updated, we monitor the value of a dummy postprocessor
sent by the master application to NekRS. In other words, we define a postprocessor
in the master application that just has a value of 1.

!listing /tutorials/sfr_7pin/solid.i
  block=synchronize

We define this postprocessor as a [Receiver](Receiver.md)
postprocessor, but we won't actually use it to receive anything from other applications.
Instead, we set the `default` value to 1 in order to indicate "true". Then,
at the same time that we send *new* flux values to NekRS, we also pass
this postprocessor.

!listing /tutorials/sfr_7pin/solid.i
  block=synchronize_in

We then receive this postprocessor in the sub-application.
This basically means that, when the flux data is *new*, the NekRS sub-application
will receive a value of "true" from the master-application (through the lens of
this postprocessor), and send the data into NekRS.
For data transfer *out* of NekRS, we determine when the temperatue data
is ready for use by MOOSE by monitoring how close the sub-application is to the
synchronization time to the master application.

All that is required to use this reduced communication
feature are to define the dummy postprocessor
in the master application, and transfer it to the sub-application. Then, set the
following options in `NekRSProblem`, which here are shown minimizing the communication
going in and out of NekRS.

!listing /tutorials/sfr_7pin/nek.i
  block=Problem

!alert warning
When the `interpolate_transfers = true` option is used
by the [TransientMultiApp](TransientMultiApp.md),
MOOSE interpolates the heat flux that gets sent to NekRS
for each NekRS time step based on the master application time steps bounding the NekRS
step. That is, if MOOSE computes the heat flux at two successive time steps to be
$q^{''}(t)$ and $q^{''}(t+1)$, and NekRS is being advanced to a sub-cycled step
$t+0.4$, then if `interpolate_transfers = true`, the `avg_flux` variable actually is
a linear interpolation of the two flux values at the end points of the master application's
solve interval, or $0.6q^{''}(t)+0.4q^{''}(t+1)$.
Using this "minimal transfer" feature will *ignore* the fact that MOOSE is
interpolating the heat flux.

!syntax parameters /Problem/NekRSProblem

!syntax inputs /Problem/NekRSProblem
