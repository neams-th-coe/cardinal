# NekRSProblem

This class performs all activities related to solving NekRS as a MOOSE application.
This class also facilitates data transfers to/from [MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
in order to exchange field data between NekRS and MOOSE. This class allows NekRS to be
coupled to MOOSE in several different manners:

- Boundary coupling via [!ac](CHT),
- Volume coupling via temperature and heat source feedback, or a
- Combination of the above

!include nek_classes.md

The smallest possible MOOSE-wrapped input file that can be used to run NekRS
is shown below. `casename` is the prefix describing the NekRS input files,
i.e. this parameter would be `casename = 'fluid'` if the NekRS input files
are `fluid.re2`, `fluid.par`, `fluid.udf`, and `fluid.oudf`.

!listing smallest_input.i
  id=smallest
  caption=Smallest possible  NekRS wrapped input file.

The remainder of this page describes how `NekRSProblem` wraps NekRS as a MOOSE application.

## Initializing MOOSE Fields

When initializing a coupling of NekRS within the MOOSE framework, the first action
taken by `NekRSProblem` is to initialize MOOSE-type variables
([MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html))
and [Postprocessors](https://mooseframework.inl.gov/syntax/Postprocessors/index.html) needed
to communicate NekRS's solution with a general MOOSE application.

First, `NekRSProblem` initializes
[MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
to receive and write spatial data necessary for multiphysics coupling. Depending on the settings for this class,
the following variables will be added:

- `temp`, the NekRS temperature to be sent to MOOSE (this is created for both boundary and volume coupling)
- `avg_flux`, the MOOSE surface heat flux to be sent to NekRS (this is created for boundary coupling)
- `heat_source`, the MOOSE heat source to be sent to NekRS (this is created for volume coupling)

Here, "boundary coupling" refers to the case when the `boundary` parameter is
specified for the [NekRSMesh](/mesh/NekRSMesh.md), while "volume coupling" refers to
the case when `volume = true` is specified for the `NekRSMesh`.
The polynomial order of each of the variables listed above is set to match the `order` selected in the
`NekRSMesh`.

This initialization of MOOSE variables happens "behind the scenes" - for instance, in
the Nek-wrapped input file in [smallest], we have indicated that
we are going to be coupling NekRS through
both boundary [!ac](CHT) and volumetric heat sources (because we set `volume = true`
and provided `boundary`).
Therefore, `NekRSProblem` essentially adds the following
to the input file:

!listing
[AuxVariables]
  [temp] # always added
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
creation, so you don't need to add these variables yourself.
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
Each of these functions is now described.

!listing /framework/src/problems/ExternalProblem.C
  re=void\sExternalProblem::solve.*?^}

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

Because `externalSolve` is wrapped inside two `syncSolutions` calls,
this means that for *every* NekRS time step, data is sent to and from
NekRS, even if NekRS runs with a smaller time step than the MOOSE application
to which it is coupled (i.e. if the data going *into* NekRS hasn't changed since
the last time it was sent to NekRS). A means by which to reduce some of these
(technically) unnecessary data transfers is described in [#min].

### Transfers to NekRS

In the `TO_EXTERNAL_APP` data transfer, [MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
 are read from the
[NekRSMesh](/mesh/NekRSMesh.md) mesh mirror and interpolated onto the NekRS
[!ac](GLL) points corresponding to each node using [Vandermonde matrices](vandermonde.md).
The specific data transfers going into NekRS are determined based on how the
`NekRSMesh` was constructed, i.e. whether boundary and/or volume coupling is used.

Data is "sent" into NekRS by writing into the `nrs->usrwrk` scratch space array,
which NekRS makes available within the boundary condition functions in the `.oudf` file
(on device, this array is technically called the `nrs->o_usrwrk` array).
[usrwrk_nrsp] shows the assignment of "slots" in the `nrs->usrwrk` scratch space
array with quantities written by Cardinal. Because different quantities are written into
Cardinal depending on the problem setup, if a particular slice is not needed for a
case, it will just hold zero values. That is, the *order* of the various quantities
is always the same in `nrs->usrwrk`.

!table id=usrwrk_nrsp caption=Quantities written into the scratch space array by Cardinal
| Slice | Quantity | When Will There be Non-Zero Values? | How to Access in the `.oudf` File |
| :- | :- | :- | :- |
| 0 | Boundary heat flux | if `boundary` is set on `NekRSMesh` | `bc->wrk[0 * bc->fieldOffset + bc->idM]` |
| 1 | Volumetric heat source | if `volume` is true on `NekRSMesh` | `bc->wrk[1 * bc->fieldOffset + bc->idM]` |
| 2 | Mesh x-displacement | if `moving_mesh` is true on `NekRSMesh` | `bc->wrk[2 * bc->fieldOffset + bc->idM]` |
| 3 | Mesh y-displacement | if `moving_mesh` is true on `NekRSMesh` | `bc->wrk[3 * bc->fieldOffset + bc->idM]` |
| 4 | Mesh z-displacement | if `moving_mesh` is true on `NekRSMesh` | `bc->wrk[4 * bc->fieldOffset + bc->idM]` |

The total number of slots in the scratch space that are allocated by Cardinal
is controlled with the `n_usrwrk_slots` parameter.
If you need to use extra slices in `nrs->usrwrk` for other custom user actions,
simply set `n_usrwrk_slots` to be greater than the number of slots strictly needed
for coupling. At the start of your Cardinal simulation, a table will be printed to the
screen to explicitly tell you what each slice in the scratch space holds.
Any extra slots are noted as `unused`, and are free for non-coupling use.

For example, if your case couples NekRS to MOOSE via volumes by
setting `volume = true`, but has `boundary` unset and `moving_mesh = false`,
the slice normally dedicated to storing heat flux is still allocated (because
we keep the order the same in `nrs->usrwrk`, and the flux is earlier in the array than
the volumetric heat source), but won't hold any meaningful information.
A table similar to the following
would print out at the start of your simulation. You could use slices 2 onwards
for custom purposes.

!listing id=l11 caption=Table printed at start of Cardinal simulation that describes available scratch space for a case that couples NekRS to MOOSE via volumetric power density, but not via boundaries or a moving mesh. A total of 7 slots are allocated by setting `n_usrwrk_slots` to 7
------------------------------------------------------------------
| Slice |  Quantity   |        How to Access in NekRS BCs        |
------------------------------------------------------------------
|     0 | flux        |  bc->wrk[0 * bc->fieldOffset + bc->idM]  |
|     1 | heat_source |  bc->wrk[1 * bc->fieldOffset + bc->idM]  |
|     2 | unused      |  bc->wrk[2 * bc->fieldOffset + bc->idM]  |
|     3 | unused      |  bc->wrk[3 * bc->fieldOffset + bc->idM]  |
|     4 | unused      |  bc->wrk[4 * bc->fieldOffset + bc->idM]  |
|     5 | unused      |  bc->wrk[5 * bc->fieldOffset + bc->idM]  |
|     6 | unused      |  bc->wrk[6 * bc->fieldOffset + bc->idM]  |
------------------------------------------------------------------

!alert warning
Allocation of `nrs->usrwrk` and `nrs->o_usrwrk` is done automatically by
`NekRSProblem`. If you attempt to run a NekRS input file that accesses `bc->wrk` in the
`.oudf` file *without* a Cardinal executable (i.e. using something like
`nrsmpi case 4`), then that scratch space will have to be manually allocated in
the `.udf` file, or else your input will seg fault. This use case will not be typically
encountered by most users, but if you really do want to run the NekRS input files
intended for a Cardinal case with the NekRS executable (perhaps for debugging),
we recommend simply replacing `bc->wrk` by a dummy value, such as `bc->flux = 0.0`
for the boundary heat flux use case. This just replaces a value that normally comes from MOOSE by a fixed
value. All other aspects of the NekRS case files should not require modification.

#### Boundary Transfers

If `boundary` was specified on [NekRSMesh](/mesh/NekRSMesh.md), then a heat flux
(stored in the variable `avg_flux`) and the total heat flux integral over the
boundary for normalization (stored in the postprocessor `flux_integral`) are sent
to NekRS. The heat flux is written into the first "slot" of a NekRS scratch space array
(`nrs->usrwrk`) by Cardinal.
Then, all that is required to use a heat flux transferred by MOOSE is to
apply it in the `scalarNeumannConditions` [!ac](OCCA) boundary condition.
Below, `bc->wrk` is the same as `nrs->o_usrwrk`, or the scratch space on the
device; this function applies the heat flux computed by MOOSE to the flux boundaries.

!listing /test/tests/cht/pebble/onepebble2.oudf language=cpp
  re=void\sscalarNeumannConditions.*?^}

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
points using [Vandermonde matrices](vandermonde.md).
The data transfers coming from
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

## What are the Fluxes?

There are a few different heat fluxes involved when coupling NekRS via [!ac](CHT)
to MOOSE. When you run a [!ac](CHT) calculation, for each time step you will see
something like the following printed to the screen:

!listing!
Sending heat flux to NekRS boundary 1
Normalizing total NekRS flux of 78.8581 to the conserved MOOSE value of 78.6508
!listing-end!

Here, the "total NekRS flux of ..." is the integral of `nrs->usrwrk` over the
high-order NekRS spectral element mesh. Conversely, the "conserved MOOSE value of ..."
is the total heat flux that MOOSE is setting in NekRS (which you are transferring
via a postprocessor to the `flux_integral` postprocessor in the NekRS MOOSE-wrapped
input file). These two numbers will be
different unless all of the following are true:

- The NekRS and MOOSE meshes are identical
- The NekRS polynomial order matches the MOOSE polynomial order, and for the same
  polynomial order the nodes are the same. Because NekRS is a spectral element method
  and MOOSE uses the finite element method, this criteria can only occur if NekRS and
  MOOSE are either 1st or 2nd order (because the node placement for 1st or 2nd order
  elements is the same for spectral and finite elements).
- The quadrature rule used to integrate in MOOSE is the [!ac](GLL) quadrature

Any differences between the "total NekRS flux" and the "conserved MOOSE value" will
simply arise due to differences in the integration of a field over their respective meshes.
You may see large differences if the geometry is curved, since the NekRS high-order
mesh will capture the curvature and result in very different area integrals.
We always recommend doing a sanity check on the flux sent from MOOSE to NekRS
via the MOOSE output files.

You can also monitor the heat flux in NekRS by computing $-k\nabla T$ using a
[NekHeatFluxIntegral](postprocessors/NekHeatFluxIntegral.md) postprocessor. In general, the quantity
computed by this postprocessor will *not* match the heat flux set by MOOSE because
both the finite and spectral element methods solve weak forms where Neumann boundary
conditions are only *weakly* imposed. That is, we set the heat flux but only enforce
it by driving the entire nonlinear residual to a small-enough number, so heat flux
boundary conditions are never perfectly observed like Dirichlet boundary conditions are.


## Other Features

This class mainly facilitates data transfers to and from NekRS. A number of other
features are implemented in order to enable nondimensional solutions,
improved communication, and convenient solution modifications. These are
described in this section.

### Nondimensional Solution

!include nondimensional_problem.md

### Outputting the NekRS Solution

!include output_solution.md

For example, consider a [!ac](CHT) simulation where NekRS is coupled to MOOSE
through a boundary. Normally, the [NekRSMesh](/mesh/NekRSMesh.md) contains only the
data that is used to couple NekRS to MOOSE - the NekRS wall temperature and the MOOSE
heat flux. The input file below will interpolate the NekRS pressure and velocity
solutions onto the mesh mirror. We set `volume = true` for the mesh mirror so that
the pressure and velocity are represented over the volume for visualization. Because
setting `volume = true` normally indicates that you want to couple NekRS to MOOSE by
a volumetric heat source (which we don't just for visualization purposes), we set
`has_heat_source = false` so that various error checks related to volume-based coupling
to MOOSE are skipped.

!listing test/tests/cht/pincell_p_v/nek.i

For instance, [output_sfr] shows the velocity from the NekRS field files
(left) and interpolated onto a second-order mesh mirror (right). Because this particular
example runs NekRS in a higher order than can be represented on a second-order mesh
mirror, the interpolated velocity is clearly not an exact representation of the NekRS
solution - only an interpolated version of the NekRS solution.

!media output_vel.png
  id=output_sfr
  caption=Velocity from the NekRS field files (left) and after interpolation onto a second order mesh mirror (right).
  style=width:80%;margin-left:auto;margin-right:auto;halign:center

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
