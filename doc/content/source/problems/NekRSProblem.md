# NekRSProblem

This class performs all activities related to solving NekRS as a MOOSE application.
This class also facilitates data transfers to/from [MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
in order to exchange field data between NekRS and MOOSE. The actual actions taken by this class depend
on whether NekRS is coupled via boundary [!ac](CHT), volume-based temperature and
heat source feedback, or a combination of the two. This class must be used in
conjunction with the [NekRSMesh](/mesh/NekRSMesh.md), and the parameters for the
`NekRSMesh` determine the type of coupling between NekRS and MOOSE. This class must
also be used in conjunction with the [NekTimeStepper](/timesteppers/NekTimeStepper.md).
Therefore, we recommend first reading the documentation for `NekRSMesh` and
`NekTimeStepper` before proceeding here.

The smallest possible MOOSE-wrapped input file that can be used to run NekRS
is shown below. `casename` is the prefix describing the NekRS input files,
i.e. this parameter would be `casename = 'fluid'` if the NekRS input files
are `fluid.re2`, `fluid.par`, `fluid.udf`, and `fluid.oudf`.
The crux of a NekRS wrapping is in the `NekRSProblem`,
[NekRSMesh](/mesh/NekRSMesh.md), and [NekTimeStepper](/timesteppers/NekTimeStepper.md)
classes. `NekRSProblem` controls the program execution and data transfers,
`NekRSMesh` creates a mesh mirror so that all the usual
[Transfers](https://mooseframework.inl.gov/syntax/Transfers/index.html) understand
how NekRS's solution is stored, and the `NekTimeStepper` allows NekRS to control
its time stepping.

!listing smallest_input.i
  id=smallest
  caption=Smallest possible  NekRS wrapped input file.

The remainder of this page describes how `NekRSProblem` wraps NekRS as a MOOSE application.

## Initializing MOOSE-type Field Interfaces

When initializing a coupling of NekRS within the MOOSE framework, the first action
taken by `NekRSProblem` is to initialize MOOSE-type variables
([MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html))
and [Postprocessors](https://mooseframework.inl.gov/syntax/Postprocessors/index.html) needed
to communicate NekRS's solution with a general MOOSE application.

First, `NekRSProblem` initializes
[MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
to receive and write spatial data necessary for multiphysics coupling. Depending on the settings for this class,
the following variables will be added:

- `temp`, the NekRS temperature to be sent to MOOSE (both boundary and volume coupling)
- `avg_flux`, the MOOSE surface heat flux to be sent to NekRS (boundary coupling)
- `heat_source`, the MOOSE heat source to be sent to NekRS (volume coupling)

Here, "boundary coupling" refers to the case when the `boundary` parameter is
specified for the [NekRSMesh](/mesh/NekRSMesh.md), while "volume coupling" refers to
the case when `volume = true` is specified for the `NekRSMesh`.
The polynomial order of each of these variables is set to match the `order` selected in the
`NekRSMesh`.

This initialization of MOOSE variables happens behind the scenes - for instance, in
the Nek-wrapped input file in [smallest], we have indicated that
we are going to be coupling NekRS through
both boundary [!ac](CHT) and volumetric heat sources (because we set `volume = true`
and provide `boundary`).
Therefore, `NekRSProblem` essentially adds the following
to the input file:

!listing
[AuxVariables]
  [temp] # added for both boundary and volume coupling
    family = LAGRANGE
    order = FIRST
  []
  [heat_source] # added for volume coupling
    family = LAGRANGE
    order = FIRST
  []
  [avg_flux] # added for boundary coupling
    family = LAGRANGE
    order = FIRST
  []
[]

This auxiliary variable addition happens *automatically* in order to simplify the input file
creation, so you don't need to add these variables yourself in the input file.
In addition to these auxiliary variables, `NekRSProblem` also automatically
adds several [Receiver](https://mooseframework.inl.gov/source/postprocessors/Receiver.html)
postprocessors that are used for ensuring conservation in the data transfers:

- `flux_integral`, or the total integrated heat flux to be conserved from the coupled MOOSE application (boundary coupling)
- `source_integral`, or the total integrated heat source (volumetric) to be conserved from the coupled MOOSE application (volume coupling)

Therefore, the second thing that `NekRSProblem` does is to essentially add the following
to the input file:

!listing
[Postprocessors]
  [flux_integral] # added for boundary coupling
    type = Receiver
  []
  [source_integral] # added for volume coupling
    type = Receiver
  []
[]

## Overall Calculation Methodology

`NekRSProblem` inherits from the [ExternalProblem](https://mooseframework.inl.gov/source/problems/ExternalProblem.html)
class. For each time step, the calculation proceeds according to the `ExternalProblem::solve()` function.
Data gets sent into NekRS, NekRS runs a time step, and data gets extracted from NekRS.
`NekRSProblem` mostly consists of defining the `syncSolutions` and `externalSolve` methods.

!listing /framework/src/problems/ExternalProblem.C
  re=void\sExternalProblem::solve.*?^}

Each of these functions is now described.

### External Solve
  id=solve

The actual solve of a timestep by NekRS is peformed within the
`externalSolve` method, which essentially performs the following.

!listing language=cpp
nekrs::runStep(time, dt, time_step_index);
nekrs::ocopyToNek(time + dt, time_step_index);
nekrs::udfExecuteStep(time + dt, time_step_index, is_output_step);
if (is_output_step) nekrs::outfld(time + dt);

These four functions are defined in the NekRS source code, and perform the following:

- Run a single time step
- Copy the device-side solution to the host-side (for access in various MOOSE-style postprocessors)
- Execute a [user-defined function](https://nekrsdoc.readthedocs.io/en/latest/input_files.html#udf-executestep-nrs-t-nrs-dfloat-time-int-tstep) in NekRS, `UDF_ExecuteStep`, for Nek-style postprocessing
- Write a NekRS output file

This means that for *every* NekRS time step, data is sent to and from
NekRS, even if NekRS runs with a smaller time step than the MOOSE application
to which it is coupled (i.e. if the data going *into* NekRS hasn't changed since
the last time it was sent to NekRS). A means by which to reduce some of these
(technically) unnecessary data transfers is described in [#min].

### Transfers to NekRS

In the `TO_EXTERNAL_APP` data transfer, [MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
 are read from the
[NekRSMesh](/mesh/NekRSMesh.md) mesh mirror and interpolated onto the NekRS
[!ac](GLL) points corresponding to each node using Vandermonde matrices.
The data transfers going into NekRS are determined based on how the
`NekRSMesh` was constructed, i.e. whether boundary and/or volume coupling is used.

#### Boundary Transfers

If `boundary` was specified on [NekRSMesh](/mesh/NekRSMesh.md), then a heat flux
(stored in the variable `avg_flux`) and the total heat flux integral over the
boundary for normalization (stored in the postprocessor `flux_integral`) are sent
to NekRS. The heat flux is written into the first "slot" of a NekRS scratch space array,
`nrs->usrwrk`. This scratch space is specifically designed for general user
utility, and its device-side version is accessible in NekRS's boundary condition functions on device.
Allocation of this scratch space is done automatically by `NekRSProblem`.
After writing into this scratch space, the scratch space is then copied to an
equivalent scratch space on the device.

Then, all that is required to use a heat flux transferred by MOOSE is to
apply it in the `scalarNeumannConditions` [!ac](OCCA) boundary condition.
Below, `bc->wrk` is the same as `nrs->o_usrwrk`, or the scratch space on the
device; this function applies the heat flux computed by MOOSE to the flux boundaries.

!listing /test/tests/cht/pebble/onepebble2.oudf language=cpp
  re=void\sscalarNeumannConditions.*?^}

!alert warning
Allocation of `nrs->usrwrk` and `nrs->o_usrwrk` is done automatically by
`NekRSProblem`. If you attempt to run a NekRS input file that applies heat flux
via `bc->wrk` *without* a Cardinal executable (i.e. using something like
`nrsmpi case 4`), then that scratch space will have to be manually allocated in
the `.udf` file, or else your input will seg fault. This use case will not be typically
encountered by most users, but if you really do want to run the NekRS input files
intended for a Cardinal case with the NekRS executable (perhaps for debugging),
we recommend simply replacing `bc->wrk[bc->idM]` by a dummy, non-zero flux value,
such as `bc->flux = 100.0;`. This just replaces a heat flux from MOOSE by a fixed
value. All other aspects of the NekRS case files should not require modification.

#### Volume Transfers

If `volume = true` is specified on [NekRSMesh](/mesh/NekRSMesh.md), then a volumetric heat source
(stored in the variable `heat_source`) and the total heat source integral over the
volume for normalization (stored in the postprocessor `source_integral`) are sent
to NekRS. The volumetric heat source is also written into the `nrs->usrwrk` scratch
space array - offset by an appropriate index to be the "second" slot in the scratch
space (recall that the heat flux is always written into the "first" field space in the
scratch array).

Then, all that is required to use a volumetric heat source transferred by MOOSE is to
apply it with a custom source [!ac](OCCA) kernel in the `.oudf` file. Below is an example
of a custom heat source kernel, arbitrarily named `mooseHeatSource` - this code is
executed on the [!ac](GPU), and is written in OKL, a decorated C++ kernel language.
This code loops over all the NekRS elements, loops over all the [!ac](GLL) points on
each element, and then sets the volumetric heat source equal to a heat source passed into
the `mooseHeatSource` function.

!listing /test/tests/conduction/nonidentical_volume/cylinder/cylinder.oudf language=cpp
  re=\@kernel void\smooseHeatSource.*?^}

The actual passing of the `nrs->usrwrk` scratch space (that `NekRSProblem` writes into)
occurs in the `.udf` file. In the `.udf` file, you need to define a custom function,
named artbirarily here to `userq`, with a signature that matches the user-defined source
function expected by NekRS. This function should then pass the scratch space
into the [!ac](OCCA) kernel we saw in the `.oudf` file.

!listing /test/tests/conduction/nonidentical_volume/cylinder/cylinder.udf language=cpp
  re=void\suserq.*?^}

To finish the application of the heat source from MOOSE, you will need to be sure
to "load" the custom heat source kernel and create the pointer needed by NekRS
to call that function. These two extra steps are quite small - the entire `.udf`
file required to apply a MOOSE heat source is shown below (a large part of this
file is applying initial conditions, which is unrelated to the custom heat source).

!listing /test/tests/conduction/nonidentical_volume/cylinder/cylinder.udf language=cpp

If your case involves applying a heat source to NekRS from MOOSE, you can simply
copy the [!ac](OCCA) kernel and `.udf` file build/load commands for your case - no
modifications should be required. If needed,
please consult the [NekRS documentation](https://nekrsdoc.readthedocs.io/en/latest/detailed_usage.html#setting-custom-source-terms)
for more information on applying custom sources.

!alert note
If `NekRSMesh` has both boundary and volume coupling specified, then both the
boundary and volume data transfers will occur - a volume mesh is constructed that
communciates a volumetric heat source, a volumetric temperature, and a boundary heat flux.
In NekRS's `.oudf` file, you will apply a heat flux in the `scalarNeumannConditions` function
*and* a volumetric heat source in a source [!ac](OCCA) kernel.

### Transfer from NekRS

In the `FROM_EXTERNAL_APP` data transfer, [MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
are written to
the [NekRSMesh](/mesh/NekRSMesh.md) by interpolating from NekRS's [!ac](GLL)
points using Vandermone matrices.
The data transfer coming from
NekRS are determined based on how the `NekRSMesh` was constructed, i.e. whether
boundary and/or volume coupling is used.

#### Boundary Transfers

If `boundary` was specified on [NekRSMesh](/mesh/NekRSMesh.md), then a temperature (stored in the
variable `temp`) is written by NekRS by reading from the corresponding boundary in the array storing the
passive scalars in NekRS, or `nrs->cds->S`.

#### Volume Transfers

If `volume = true` was specified on [NekRSMesh](/mesh/NekRSMesh.md), then the temperature (stored in
the variable `temp`) is written by NekRS by reading from the entire volume in
the array storing the passive scalars in NekRS, or `nrs->cds->S`. In other words,
the boundary and volume transfers are basically the same coming *from* NekRS -
the temperature is written into the `temp` variable. The entire volume data is
written for volume transfers, whereas only the temperature on the specified boundaries
is written for boundary transfers.

## Other Features

This class mainly facilitates data transfers to and from NekRS. A number of other
features are implemented in order to enable nondimensional solutions,
improved communication, and convenient solution modifications. These are
described in this section.

### Nondimensional Solution

!include nondimensional_problem.md

### Reducing CPU/GPU Data Transfers
  id=min

As shown in [#solve], for *every* NekRS time step, data is passed into NekRS
(which involves interpolating from a MOOSE mesh to a (usually) higher-order NekRS
mesh and then copying this field from host to device) and data is passed out from
NekRS (which involves copying this field from device to host and then
interpolating from a (usually) higher-order NekRS mesh to a MOOSE mesh). When NekRS
is run in standalone mode, data is only copied between the device and host when an
output file is written.

If NekRS is run as a sub-application to a master application, and sub-cycling is used,
a lot of these interpolations and [!ac](CPU)/[!ac](GPU) data transfers can be omitted.
When you run a Nek-wrapped input file, timing estimates for these data transfers are
printed to the screen. Even for NekRS meshes with a few million elements, these interpolations
to/from NekRS typically require only fractions of a second. But for some use cases, you
may want to reduce as many of these communciations as possible.

First, let's explain what MOOSE does in the usual master/sub coupling scheme, using
boundary coupling with `subcycling = true` as an example.
Suppose you have a master application with a time step size of 1 second, and run NekRS
as a sub-application with a time step size of 0.4 seconds that executes at the end of
the master application time step. The calculation procedure involves:

1. Solve the master application from $t$ to $t+1$ seconds.
1. Transfer a variable representing flux (and its integral) from the master
  application to the NekRS sub-application) at $t$.
1. Interpolate the flux from the [NekRSMesh](/mesh/NekRSMesh.md)
  onto NekRS's [!ac](GLL) points, then normalize it and copy it from host to device.
1. Run a NekRS time step from $t$ to $t+0.4$ seconds.
1. Copy the temperature from device to host and then interpolate NekRS's
  temperature from NekRS's [!ac](GLL) points to the `NekRSMesh`.
1. *Even though the flux data hasn't changed*, and
  *even though the temperature data isn't going to be used by the master application yet*,
  `ExternalProblem::solve()`
  performs data transfers in/out of NekRS for *every* NekRS time step. Regardless of
  whether that data has changed or is needed yet by MOOSE, repeat steps 3-5 two times -
  once for a time step size of 0.4 seconds, and again for a time step size of 0.2 seconds
  (for the NekRS sub-application to "catch up" to the master application's overall time
  step length of 1 second.

If NekRS is run with a time step $N$ times smaller than its master
application, this structuring of [ExternalProblem](https://mooseframework.inl.gov/source/problems/ExternalProblem.html)
 represents $N-1$ unnecessary
interpolations and [!ac](CPU) to [!ac](GPU) copies of the flux, and $N-1$
unnecessary [!ac](GPU) to [!ac](CPU) copies of the temperature and interpolations.
`NekRSProblem` contains features that allow you to turn off these extra transfers.
However, MOOSE's [MultiApp](https://mooseframework.inl.gov/syntax/MultiApps/index.html)
system is designed in such a way that sub-applications know very little about
their master applications (and for good reason - such a design is what enables such
flexible multiphysics couplings). So, the only way that NekRS can definitively
know that a data transfer from a master application is the *first* data transfer
after the flux data has been updated, we monitor the value of a dummy postprocessor
sent by the master application to NekRS. In other words, we define a postprocessor
in the master application that just has a value of 1.

!listing /tutorials/sfr_7pin/solid.i
  block=synchronize

We define this postprocessor as a [Receiver](https://mooseframework.inl.gov/source/postprocessors/Receiver.html)
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
by the [TransientMultiApp](https://mooseframework.inl.gov/source/multiapps/TransientMultiApp.html),
MOOSE interpolates the heat flux that gets sent to NekRS
for each NekRS time step based on the master application time steps bounding the NekRS
step. That is, if MOOSE computes the heat flux at two successive time steps to be
$q^{''}(t)$ and $q^{''}(t+1)$, and NekRS is being advanced to a sub-cycled step
$t+0.4$, then if `interpolate_transfers = true`, the `avg_flux` variable actually is
a linear interpolation of the two flux values at the end points of the master application's
solve interval, or $0.6q^{''}(t)+0.4q^{''}(t+1)$.
Using this "minimal transfer" feature will *ignore* the fact that MOOSE is
interpolating the heat flux.

### Limiting Temperature

For many NekRS simulations, such as those with sharp interior corners, it is often of
use to "clip" the temperature to prevent oscillations. The temperature extracted from
NekRS can be clipped before being transferred to a coupled MOOSE application by providing
either one of both of
the `min_T` and `max_T` postprocessors. With these specified, the temperature written to
the [NekRSMesh](/mesh/NekRSMesh.md) is adjusted to the range $\left\lbrack T_{min},T_{max}\right\rbrack$.
`min_T` and `max_T` should be given in dimensional units.

!syntax parameters /Problem/NekRSProblem

!syntax inputs /Problem/NekRSProblem

!syntax children /Problem/NekRSProblem
