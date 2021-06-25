# NekRSProblem

This class performs all activities related to solving nekRS as a MOOSE application.
This class also facilitates data transfers to/from [MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
to exchange field data from/to nekRS. The actual actions taken by this class depend
on whether nekRS is coupled via boundary [!ac](CHT), volume-based temperature and
heat source feedback, or some combination of these two.

The smallest possible MOOSE-wrapped input file that can be used to run nekRS
is shown below. The crux of a nekRS wrapping is in the `NekRSProblem`,
[NekRSMesh](/mesh/NekRSMesh.md), and [NekTimeStepper](/timesteppers/NekTimeStepper.md)
classes. `NekRSProblem` controls the program execution and data transfers,
`NekRSMesh` creates a mesh mirror so that all the usual
[Transfers](https://mooseframework.inl.gov/syntax/Transfers/index.html) understand
how nekRS's solution is stored, and the `NekTimeStepper` allows nekRS to control
its time stepping. Before reading this page, please first visit the mesh and
time stepper descriptions.

!listing smallest_input.i

## Initializing MOOSE-type Field Interfaces

When beginning a coupling of nekRS within the MOOSE framework, the first action
taken by this class is to initialize MOOSE-type variables and postprocessor needed
to communicate nekRS's solution with a general MOOSE application.

First, `NekRSProblem` initializes
[MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
to receive data necessary for multiphysics coupling. Depending on the settings for this class,
the following variables will be added:

- `temp`, the nekRS temperature to be sent to MOOSE (both boundary and volume coupling)
- `avg_flux`, the MOOSE surface heat flux to be sent to nekRS (boundary coupling)
- `heat_source`, the MOOSE heat source to be sent to nekRS (volume coupling)

The order of each of these variables is set to match the `order` selected in the
[NekRSMesh](/mesh/NekRSMesh.md), or the mesh mirror through which data transfers occur.
This initialization of MOOSE variables happens behind the scenes - for instance, below
is a complete input file that will run nekRS as a MOOSE application.

!listing /test/tests/conduction/boundary_and_volume/prism/nek.i

In this particular example, we indicated that we are going to be coupling nekRS through
both boundary [!ac](CHT) and volumetric heat sources (because we set `volume = true`
and provide `boundary` - see [NekRSMesh](/mesh/NekRSMesh.md) for more information).
Therefore, the first thing that `NekRSProblem` does is to essentially add the following
to the input file:

!listing
[AuxVariables]
  [temp]
    family = LAGRANGE
    order = FIRST
  []
  [heat_source]
    family = LAGRANGE
    order = FIRST
  []
  [avg_flux]
    family = LAGRANGE
    order = FIRST
  []
[]

This auxiliary variable addition happens automatically to simplify the input file
creation. In addition to these auxiliary variables, `NekRSProblem` also automatically
adds several [Receiver](https://mooseframework.inl.gov/source/postprocessors/Receiver.html)
postprocessors that are used for ensuring conservation in the data transfers:

- `flux_integral`, or the total integrated heat flux to be conserved from the coupled MOOSE application (boundary coupling)
- `source_integral`, or the total integrated heat source (volumetric) to be conserved from the coupled MOOSE application (volume coupling)

Therefore, the second thing that `NekRSProblem` does is to essentially add the following
to the input file:

!listing
[Postprocessors]
  [flux_integral]
    type = Receiver
  []
  [source_integral]
    type = Receiver
  []
[]

## Overall Calculation Methodology

`NekRSProblem` inherits from the [ExternalProblem](https://mooseframework.inl.gov/source/problems/ExternalProblem.html)
class. For each time step, the calculation proceeds according to the `ExternalProblem::solve()` function.
Data gets sent into nekRS, nekRS runs a time step, and data gets extracted from nekRS.
`NekRSProblem` mostly consists of defining the `syncSolutions` and `externalSolve` methods.

!listing /framework/src/problems/ExternalProblem.C
  re=void\sExternalProblem::solve.*?^}

### External Solve
  id=solve

The actual solve of a timestep by nekRS is peformed within the
`externalSolve` method, which essentially performs the following.

!listing
nekrs::runStep(time, dt, time_step_index);
nekrs::ocopyToNek(time + dt, time_step_index);
nekrs::udfExecuteStep(time + dt, time_step_index, is_output_step);
if (is_output_step) nekrs::outfld(time + dt);

These four functions are defined in the nekRS source code, and essentially perform:

- Run a single time step
- Copy the device-side solution to the host-side (for access in various MOOSE-style postprocessors)
- Execute a [user-defined function](https://nekrsdoc.readthedocs.io/en/latest/input_files.html#udf-executestep-nrs-t-nrs-dfloat-time-int-tstep) in nekRS, `UDF_ExecuteStep`, for Nek-style postprocessing
- Write a nekRS output file

This means that for *every* nekRS time step, data is sent to and from
nekRS, even if nekRS runs with a smaller time step than the MOOSE application
to which it is coupled (i.e. if the data going *into* nekRS hasn't changed since
the last time it was sent to nekRS). A means by which to reduce some of these
(technically) unnecessary data transfesr is described in [#min].

## Transfers to nekRS

In the `TO_EXTERNAL_APP` data transfer, `MooseVariables` are read from the
[NekRSMesh](/mesh/NekRSMesh.md) mesh mirror and interpolated onto the nekR
[!ac](GLL) points corresponding to each node. Vandermonde matrices are used
to interpolate from the nodes in the `NekRSMesh` to nekRS's [!ac](GLL) points.
The data transfers to go into nekRS are determined based on how the
`NekRSMesh` was constructed.

### Boundary Transfers

If `boundary` was specified on `NekRSMesh`, then a heat flux
(stored in the variable `avg_flux`) and the total heat flux integral over the
boundary for normalization (stored in the postprocessor `flux_integral`) are sent
to nekRS. The heat flux is written into a nekRS scratch space array,
`nrs->usrwrk`. This scratch space is specifically designed for general user
utility, and its device-side version is accessible in nekRS's boundary condition functions on device.
Initialization of this scratch space is done automatically by `NekRSProblem`.
After writing into this scratch space, the scratch space is then copied to an
equivalent scratch space on the device.

Then, all that is required to use a heat flux transferred by MOOSE is to
apply it in the `scalarNeumannConditions` [!ac](OCCA) boundary condition.
Below, `bc->wrk` is the same as `nrs->o_usrwrk`, or the scratch space on the
device; this function applies the heat flux computed by MOOSE to the flux boundaries.

!listing /test/tests/cht/pebble/onepebble2.oudf
  re=void\sscalarNeumannConditions.*?^}

!alert warning
Allocation for `nrs->usrwrk` and `nrs->o_usrwrk` is done automatically by
`NekRSProblem`. If you attempt to run a nekRS input file that applies heat flux
via `bc->wrk` *without* a Cardinal executable (i.e. using something like
`nrsmpi case 4`), then that scratch space will have to be manually allocated in
the `.udf` file, or else your input will seg fault.

### Volume Transfers

If `volume = true` is specified on `NekRSMesh`, then a volumetric heat source
(stored in the variable `heat_source`) and the total heat source integral over the
volume for normalization (stored in the postprocessor `source_integral`) are sent
to nekRS. The volumetric heat source is also written into the `nrs->usrwrk` scratch
space array - offset by an appropriate index to be the "second" field in the scratch
space (recall that the heat flux is always written into the "first" field space in the
scratch array).

Then, all that is required to use a volumetric heat source transferred by MOOSE is to
apply it with a custom source [!ac](OCCA) kernel in the `.oudf` file. Below is an example
of a custom heat source kernel, arbitrarily named `mooseHeatSource` - this code is
executed on the [!ac](GPU), and is written in OKL, a decorated C++ kernel language.
This code loops over all the nekRS elements, loops over all the [!ac](GLL) points on
each element, and then sets the volumetric heat source equal to a heat source passed into
the `mooseHeatSource` function.

!listing /test/tests/conduction/nonidentical_volume/cylinder/cylinder.oudf
  re=\@kernel void\smooseHeatSource.*?^}

The actual passing of the `nrs->usrwrk` scratch space (that `NekRSProblem` writes into)
occurs in the `.udf` file. In the `.udf` file, you need to define a custom function,
named artbirarily here to `userq`, with a signature that matches the user-defined source
function expected by nekRS. This function should then pass the scratch space
into the [!ac](OCCA) kernel we saw in the `.oudf` file.

!listing /test/tests/conduction/nonidentical_volume/cylinder/cylinder.udf
  re=void\suserq.*?^}

To finish the application of the heat source from MOOSE, you will need to be sure
to "load" the custom heat source kernel and create the pointer needed by nekRS
to call that function. These two extra steps are quite small - the entire `.udf`
file required to apply a MOOSE heat source is shown below (a large part of this
file is applying initial conditions, which is unrelated to the custom heat source).

!listing /test/tests/conduction/nonidentical_volume/cylinder/cylinder.udf

Please consult the [nekRS documentation](https://nekrsdoc.readthedocs.io/en/latest/detailed_usage.html#setting-custom-source-terms)
for more information on applying custom soruces.

!alert note
If `NekRSMesh` has both boundary and volume coupling specified, then both the
boundary and volume data transfers will occur - a volume mesh is constructed that
communciates a volumetric heat source and temperature and a boundary heat flux.

## Transfer from nekRS

In the `FROM_EXTERNAL_APP` data transfer, `MooseVariables` are written to on
the [NekRSMesh](/mesh/NekRSMesh.md) by interpolating from nekRS's [!ac](GLL)
points. Vandermonde matrices are again used to interpolate from the quadrature
points in nekRS to the nodes in the mesh mirror. The data transfer coming from
nekRS are determined based on how the `NekRSMesh` was constructed.

### Boundary Transfers

If `boundary` was specified on `NekRSMesh`, then a temperature (stored in the
variable `temp`) is written by nekRS by reading from the corresponding boundary in the array storing the
passive scalars in nekRS, or `nrs->cds->S`.

### Volume Transfers

If `volume = true` was specified on `NekRSMesh`, then the temperature (stored in
the variable `temp`) is written by nekRS by reading from the entire volume in
the array storing the passive scalars in nekRS, or `nrs->cds->S`. In other words,
the boundary and volume transfers are basically the same coming *from* nekRS -
the temperature is written into the `temp` variable. The entire volume data is
written for volume transfers, whereas only the temperature on the specified boundaries
is written for boundary transfers.

## Other Features

This class mainly facilitates data transfers to/and from nekRS. A number of other
features are implemented in order to enable nondimensional solutions,
improved communication, and convenient solution modifications. These are
described in this section.

### Nondimensional Solution

nekRS is most often solved in nondimensional form, such that all solution variables
are of order unity by normalizing by problem-specific characteristic scales. However,
most other MOOSE applications use dimensional units; when transferring data to/from
nekRS, it is important that all data transfers *out* of nekRS be properly dimensionalized
before being used in another MOOSE application. Likewise, it is important that all
data transfers *into* nekRS be properly nondimensionalized to match the nondimensional
formulation. `NekRSProblem` automatically performs these scaling operations for you, as
well as dimensionalizing the various Nek postpocessors in Cardinal.

If your nekRS input files are in nondimensional form, you must set
`nondimensional = true` and provide the various characteristic scales that were used to
set up the nekRS inputs. Cardinal assumes that the nekRS inputs were nondimensionalized
with the following:

\begin{equation}
\label{eq:u_ref}
u_i^\dagger\equiv\frac{u_i}{u_{ref}}
\end{equation}

\begin{equation}
\label{eq:p_ref}
P^\dagger\equiv\frac{P}{\rho_0u_{ref}^2}
\end{equation}

\begin{equation}
\label{eq:T_ref}
T^\dagger\equiv\frac{T-T_{ref}}{\Delta T}
\end{equation}

\begin{equation}
\label{eq:x_ref}
x_i^\dagger\equiv\frac{x_i}{L_{ref}}
\end{equation}

\begin{equation}
\label{eq:t_ref}
t^\dagger\equiv\frac{t}{L_{ref}/u_{ref}}
\end{equation}

where $\dagger$ superscripts indicate nondimensional quantities. In `NekRSProblem`,
`U_ref` is used to specify $u_{ref}$, `T_ref` is used to specify $T_{ref}$,
`dT_ref` is used to specify $\Delta T$, `L_ref` is used to specify $L_{ref}$,
`rho_0` is used to specify $\rho_0$, and `Cp_0` is used to specify $C_{p,0}$
(which does not appear above, but is necessary for scaling a volumetric heat source).
Finally, the mesh mirror must be in the same units as used in the coupled MOOSE application,
so the `scaling` parameter on [NekRSMesh](/mesh/NekRSMesh.md) must be set to
dimensionalize the nondimensional `.re2` mesh. In other words,
`scaling` must be set to $1/L_{ref}$.

!alert warning
These characteristic scales are used by Cardinal to scale the nekRS solution
into the units that the coupled MOOSE application expects. *You* still need to properly
non-dimensionalize the nekRS input files. That is, you cannot
simply specify the non-dimensional scales in `NekRSProblem` and expect a *dimsensional*
nekRS input specification to be converted to non-dimensional form.

### Reducing CPU/GPU Data Transfers
  id=min

As shown in [#solve], for *every* nekRS time step, data is passed into nekRS
(which involves interpolating from a MOOSE mesh to a (usually) higher-order nekRS
mesh and then copying this field from host to device) and data is passed out from
nekRS (which involves copying this field from device to host and then
interpolating from a (usually) higher-order nekRS mesh to a MOOSE mesh). When nekRS
is run in standalone mode, data is only copied between the device and host when an
output file is written.

If nekRS is run as a sub-application to a master application, and sub-cycling is used,
a lot of these interpolations and [!ac](CPU)/[!ac](GPU) data transfers can be omitted.
First, let's explain what MOOSE does in the usual master/sub coupling scheme, using
boundary coupling with `subcycling = true` as an example.

Suppose you has a master application with a time step size of 1 second, and run nekRS
as a sub-application with a time step size of 0.4 seconds that executes at the end of
the master application time step. The calculation procedure involves:

1. Solve the master application from $t$ to $t+1$ seconds.
1. Transfer a variable representing flux (and its integral) from the master
  application to the [NekApp](/base/NekApp.md) sub-application) at $t$.
1. Interpolate the flux from the [NekRSMesh](/mesh/NekRSMesh.md)
  onto nekRS's [!ac](GLL) points, then normalize it and copy it from host to device.
  This is simply `syncSolutions(TO_EXTERNAL_APP)`.
1. Run a nekRS time step from $t$ to $t+0.4$ seconds.
1. Copy the temperature from device to host and then interpolate nekRS's
  temperature from nekRS's [!ac](GLL) points to the `NekRSMesh`. This is
  simply `syncSolutions(FROM_EXTERNAL_APP)`.
1. *Even though the flux data hasn't changed*, and
  *even though the temperature data isn't going to be used by the master application yet*,
  `ExternalProblem::solve()`
  performs data transfers in/out of nekRS for *every* nekRS time step. Regardless of
  whether that data has changed or is needed yet by MOOSE, repeat steps 3-5 two times -
  once for a time step size of 0.4 seconds, and again for a time step size of 0.2 seconds
  (for the nekRS sub-application to "catch up" to the master application's overall time
  step length of 1 second.

If nekRS is run with a much smaller time step size $N$ times smaller than its master
application, this structuring of `ExternalProblem` represents $N-1$ unnecessary
interpolations and [!ac](CPU) to [!ac](GPU) copies of the flux, and $N-1$
unnecessary [!ac](GPU) to [!ac](CPU) copies of the temperature and interpolations.
`NekRSProblem` contains features that allow you to turn off these extra transfers.
However, MOOSE's [MultiApp](https://mooseframework.inl.gov/syntax/MultiApps/index.html)
system is designed in such a way that sub-applications know very little about
their master applications (and for good reason - such a design is what enables such
flexible multiphysics coulpings). So, the only way that `NekApp` can definitively
know that a data transfer from a master application is the *first* data transfer
after the flux data has been updated, we monitor the value of a dummy postprocessor
sent by the master application to `NekApp`. In other words, we define a postprocessor
in the master application that just has a value of 1.

!listing /tutorials/sfr_7pin/solid.i
  block=synchronize

We define this postprocessor as a [Receiver](https://mooseframework.inl.gov/source/postprocessors/Receiver.html)
postprocessor, but we won't actually use it to receive anything from other applications.
Instead, we set the `default` value to 1 in order to indicate "true". Then,
at the same time that we send *new* flux values to `NekApp`, we also pass
this postprocessor.

!listing /tutorials/sfr_7pin/solid.i
  block=synchronize_in

We then receive this postprocessor in the sub-application.

!listing /tutorials/sfr_7pin/nek.i
  block=synchronize

This basically means that, when the flux data is *new*, the `NekApp` sub-application
will receive a value of "true" from the master-application (through the lens of
this postprocessor).

For data transfer *out* of nekRS, we determine when the temperatue data
is ready for use by MOOSE by monitoring how close the sub-application is to the
synchronization time to the master application.

All that is required to use this feature are to define the dummy postprocessor
in the master application, and transfer it to the sub-application. Then, set the
following options in `NekRSProblem`, where `transfer_in` is the name of the
receiving postprocessor in the sub-application.

!listing /tutorials/sfr_7pin/nek.i
  block=Problem

!alert warning
When the `interpolate_transfers = true` option is used
by the [TransientMultiApp](https://mooseframework.inl.gov/source/multiapps/TransientMultiApp.html),
MOOSE interpolates the heat flux that gets sent to nekRS
for each nekRS time step based on the master application time steps bounding the nekRS
step. Using this "minimal transfer" feature will *ignore* the fact that MOOSE is
interpolating the heat flux.

### Limiting Temperature

For many nekRS simulations, such as those with sharp interior corners, it is often of
use to "clip" the temperature to prevent oscillations. The temperature extracted from
nekRS can be clipped before being transferred to a coupled MOOSE application by providing
either one of both of
the `min_T` and `max_T` postprocessors. With these specified, the temperature written to
the [NekRSMesh](/mesh/NekRSMesh.md) is adjusted to the range $\left\lbrack T_{min},T_{max}\right\rbrack$.

!syntax parameters /problems/NekRSProblem

!syntax inputs /problems/NekRSProblem

!syntax children /problems/NekRSProblem
