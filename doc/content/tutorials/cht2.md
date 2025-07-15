# Conjugate Heat Transfer for Laminar Pin Bundle Flow

In this tutorial, you will learn how to:

- Couple NekRS with MOOSE for [!ac](CHT) in a 7-pin bundle
- Use different [!ac](CHT) boundary conditions
- Control how flux normalization is performed in NekRS (by either lumping all sidesets together, or preserving for each sideset individually)
- Reduce the amount of copy to/from commands between host and device for NekRS
  (an advanced user feature)

To access this tutorial,

```
cd cardinal/tutorials/sfr_7pin
```

This tutorial also requires you to download some mesh files from Box.
Please download the files from the `sfr_7pin` folder
[here](https://anl.app.box.com/s/irryqrx97n5vi4jmct1e3roqgmhzic89/folder/141527707499)
and place these files within the same directory structure in
`tutorials/sfr_7pin`.

## Geometry and Computational Model

The geometry
is a shorter, 7-pin version of the fuel bundles in the [!ac](ABTR) [!cite](abtr).
Relevant dimensions are summarized in
[table1].

!table id=table1 caption=Geometric and operating conditions for the [!ac](ABTR), based on [!cite](abtr)
| Parameter | Value |
| :- | :- |
| Pellet diameter | 6.03 mm |
| Clad diameter | 8.00 mm |
| Clad thickness | 0.52 mm |
| Height | 0.2032 m |
| Flat-to-flat distance inside duct | 0.02625 m |
| Bundle power | 21 kW |
| Core inlet temperature | 355&deg;C |
| Mass flowrate | 0.1 kg/s |
| Coolant | sodium |

### Heat Conduction Model

The MOOSE heat transfer module is used to solve for [energy conservation in the solid](theory/heat_eqn.md).
The outer surface of the duct and the tops and bottoms of the
pins and ducts are insulated. In this tutorial, we will demonstrate two different
coupling options with NekRS -- for NekRS to send a convective heat flux to the solid
(the "flux-flux" coupling option) or for NekRS to send a wall temperature to
the solid (the "flux-temperature" coupling option). We will first describe the
"flux-temperature" option, and only show the input file modifications needed to
achieve the "flux-flux" coupling towards the end in [#fluxflux].

The gap region between the pellet and the cladding is unmeshed, and a quadrature-based
thermal contact model is applied based on the sum of thermal conduction and thermal radiation
(across a transparent medium).
For a paired set of boundaries,
each quadrature point on boundary $A$ is paired with the nearest quadrature point on boundary B.
Then, the sum of the radiation and conduction heat fluxes imposed between pairs of
quadrature points is

\begin{equation}
\label{eq:1}
q^{''}=\sigma\frac{T^4-T_{gap}^4}{\frac{1}{\sigma_A}+\frac{1}{\sigma_B}-1}+\frac{T-T_{gap}}{r_{th}}
\end{equation}

where $\sigma$ is the Stefan-Boltzmann constant, $T$ is the temperature at a quadrature
point, $T_{gap}$ is the temperature of the nearest quadrature point across the gap,
$\sigma_A$ and $\sigma_B$ are emissivities of boundaries $A$ and $B$, respectively, and
$r_{th}$ is the conduction resistance. For cylinders, the conduction is

\begin{equation}
\label{eq:2}
r_{th}=\frac{\ln{\left(\frac{r_2}{r_1}\right)}}{2\pi L k}
\end{equation}

where $r_2>r_1$ are the outer and inner radii
of the cylindrical annulus, $L$ is the height of the annulus, and $k$ is the
thermal conductivity of the annulus material.

Both the solid temperature and the surface temperature boundary condition (that will
be provided by NekRS) are set to an initial condition of 500 K.

### NekRS Model

NekRS is used to solve the [incompressible Navier-Stokes equations](theory/ins.md).
The inlet velocity
is selected such that the mass flowrate is 0.1 kg/s, which is low enough that the flow is
laminar and a turbulence model is not required for this example.
At the outlet, a zero (gage) pressure is imposed and an outflow condition is applied for
the energy conservation equation. On all solid-fluid interfaces, the velocity is set
to the no-slip condition and a heat flux is imposed in the energy conservation equation
(computed by MOOSE).

The initial pressure is set to zero. Both the velocity and temperature
are set to uniform initial conditions that match the inlet conditions.

## Meshing

### Solid Mesh

MOOSE [MeshGenerators](Mesh/index.md) are used to generate the meshes for the solid.
The same solid mesh will be used in the various different models in this tutorial,
so we place the content to generate the mesh in `mesh.i`, and then include it
from within the other files.

!listing /tutorials/sfr_7pin/mesh.i

!listing /tutorials/sfr_7pin/solid.i
  end=Variables

The solid mesh is shown in [solid_mesh]; the different regions in this mesh are first
created in 2-D, and then extruded into 3-D. You can visualize the mesh when you
run the simulations (in the output files), or you can generate it without running
the simulation (if you wanted to view it separately from a simulation) by doing

!listing
cardinal-opt -i solid.i --mesh-only

which will create the mesh as `solid_in.e`. The boundary names are illustrated towards
the right by showing the highlighted surface to which each boundary corresponds.
A unique block ID is used for the set of elements in the cladding,
the duct, and for the pellet elements. Two block IDs are required to describe the pellet
regions because two different element types (quads and prisms) are present in the pellet region.
The pellet-clad gap is unmeshed.

!media sfr_solid_mesh.png
  id=solid_mesh
  caption=Mesh for the solid portions of the 7-pin bare [!ac](SFR) bundle
  style=halign:center

Unique boundary names are set for each boundary to which we will apply a unique boundary
condition. Because we will apply insulated boundary conditions on the top and bottom
surfaces, as well as on the outside of the duct, we don't need to define sidesets (
the "do-nothing" boundary condition in the finite element method is a zero-flux condition).

### Fluid Mesh

The complete fluid mesh is shown in [fluid_mesh]; this is first created using meshing
software, and then converted into a NekRS mesh (`.re2` format).
The boundary names are illustrated
by showing the highlighted surface to which each corresponds.
Note that the meshes in [solid_mesh] and [fluid_mesh]
do not need to share nodes between the fluid and solid
boundaries through which [!ac](CHT) coupling will be performed - interpolations
using MOOSE's [Transfers](Transfers/index.md) handle any differences in the mesh.

!media sfr_fluid_mesh.png
  id=fluid_mesh
  caption=Mesh for the fluid portions of the 7-pin bare [!ac](SFR) bundle
  style=halign:center

## Flux-Temperature CHT Coupling
  id=fluxtemperature

In this section, NekRS and MOOSE are coupled for [!ac](CHT) using the "flux-temperature" coupling mode.

### Solid Input Files

The solid phase is solved with the MOOSE heat transfer module, and is described
in the `solid.i` input file. At the top of this file, various problem parameters
are defined as file-local variables to help with setting up the uniform heat
source in the fuel.

!listing tutorials/sfr_7pin/solid.i
  end=Variables

The heat transfer module will solve for temperature, which is defined as a nonlinear
variable.

!listing tutorials/sfr_7pin/solid.i
  block=Variables

The [Transfer](Transfers/index.md) system is used to communicate variables across applications; a boundary
heat flux will be computed by MOOSE and applied as a boundary condition in NekRS. In the
opposite direction, NekRS will compute a surface temperature that will be applied as a
boundary condition in MOOSE. Therefore, we define auxiliary variables to hold the flux
computed by MOOSE (`flux`) and the surface temperature received from NekRS
(`nek_temp`).

!listing tutorials/sfr_7pin/solid.i
  block=AuxVariables

Next, the governing equation solved by MOOSE is specified with the `Kernels` block as
the [HeatConduction](HeatConduction.md)
 kernel with a volumetric heat source in the pellets with the
[BodyForce](BodyForce.md) kernel.
Notice how we can do math with the file-local variables that were defined at the
top of the file, with `${fparse <math statement>}` syntax.

!listing tutorials/sfr_7pin/solid.i
  block=Kernels

For computing the heat flux on the boundaries coupled to NekRS (the clad outer surface
and the duct inner surface), we use a [DiffusionFluxAux](DiffusionFluxAux.md).

!listing tutorials/sfr_7pin/solid.i
  block=AuxKernels

The [HeatConductionMaterial](HeatConductionMaterial.md)
is then used to specify functional-forms for the thermal conductivity
of the pellet, clad, and duct. For the [HeatConductionMaterial](HeatConductionMaterial.md), you can use `t` in [ParsedFunctions](MooseParsedFunction.md)
to represent temperature.

!listing tutorials/sfr_7pin/solid.i
  start=Functions
  end=Postprocessors

Next, we define boundary conditions for the solid. Between the pellet surface
and the clad inner surface, we impose a thermal contact model as described in
[eq:1]. On fluid-solid interfaces, the solid temperature is set equal to
the surface temperature computed by NekRS.

!listing tutorials/sfr_7pin/solid.i
  start=ThermalContact
  end=Functions

Next, the [MultiApps](MultiApps/index.md) and [Transfers](Transfers/index.md)
blocks describe the interaction between Cardinal
and MOOSE. The MOOSE heat transfer module is here run as the main application, with
the NekRS wrapping run as the sub-application.
Three transfers are required to couple Cardinal and MOOSE; the first is a transfer
of surface temperature from Cardinal to MOOSE.
The second is a transfer of heat flux from
MOOSE to Cardinal. And the third is a transfer of the total integrated heat flux from MOOSE
to Cardinal (computed as a postprocessor), which is then used internally by NekRS to re-normalize the heat flux (after
interpolation onto its [!ac](GLL) points).

!listing tutorials/sfr_7pin/solid.i
  start=MultiApps
  end=AuxVariables

In addition to these three transfers necessary to couple NekRS with MOOSE,
there is a fourth transfer - `synchronize_in`, which transfers the `synchronize`
postprocessor to NekRS. This is an *optional* transfer and is only used for
performance reasons to reduce the number of data transfers.
The `synchronize` postprocessor
is simply a [Receiver](Receiver.md)
postprocessor that is set to a value of 1. No applications will transfer anything
*in* to `synchronize`, so the value of this postprocessor remains always fixed
at 1. In addition to the `synchronize` postprocessor, below are listed other
postprocessors used to facilitate the data transfers and output certain quantities
of interest.

!listing tutorials/sfr_7pin/solid.i
  block=Postprocessors

To understand the purpose of this (optional) transfer, we need to describe in more
detail the data transfers that occur when sub-cycling. Please note that this is an
advanced feature added for very large runs to squeeze out as much performance as possible -
understanding this feature is not necessary for beginner users.

Consider the case where the main application has a time step of 1,
but NekRS has a time step of 0.2. After the solution of the main application, the heat flux
transfer into NekRS consists of several steps:

1. Transfer from `flux` in the main application to the `heat_flux` receiver variable in
   the MOOSE-wrapped NekRS app.
   This is the transfer that happens in the `Transfers` block.
2. Once the heat flux is available in the `heat_flux` variable in the `nek.i` input, transfer
   that heat flux into NekRS on the host (i.e. the CPU) by interpolating from the
   [NekRSMesh](NekRSMesh.md) to the [!ac](GLL) points. Also normalize the heat flux using the `heat_flux_integral` postprocesor to ensure conservation.
3. Once the heat flux has been normalized on the host, it is then copied from the host to the device (i.e. the parallel backend,
   which will be either a CPU or GPU).

If NekRS is run with a much smaller time step than the main application,
steps 2 and 3 can be omitted to save on the interpolation from the [NekRSMesh](NekRSMesh.md)
to NekRS's [!ac](GLL) points *and* on the copy from the host to the device.
However, MOOSE's design means that the sub-application doesn't really know anything
about how it fits into the hierarchical multiapp tree - it is agnostic. So, the user
of this postprocessor (plus some settings in [NekRSProblem](NekRSProblem.md), to be
discussed shortly) can be used to only perform steps 2 and 3 *only
on the synchronization points* between NekRS and MOOSE. In other words,
if NekRS runs with a time step 100 times smaller than a main application,
this feature reduces the mesh interpolation
and host-to-device copying by a factor of 100. By transferring this "dummy"
postprocessor to the NekRS wrapping, NekRS will have a signal of when
the synchronization points occur.

Finally, we specify an executioner and an exodus output for the solid solution.

!listing tutorials/sfr_7pin/solid.i
  start=Executioner

### Fluid Input Files

The fluid phase is solved with NekRS, and is specified
in the `nek.i` file. For [!ac](CHT) coupling, first we construct a mirror of NekRS's
mesh on the boundaries of interest - the IDs associated with the fluid-solid interfaces
(as known to NekRS) are boundaries 1 and 2.

!listing tutorials/sfr_7pin/nek.i
  block=Mesh

Next, [NekRSProblem](NekRSProblem.md) is used to describe all aspects of the
NekRS wrapping.

!listing tutorials/sfr_7pin/nek.i
  block=Problem

We use `synchronization_interval = parent_app` to
indicate that we want to transfer data into NekRS's `.oudf` backend only when new coupling data is
available from the parent application. When this option is used,
[NekRSProblem](NekRSProblem.md) automatically adds a [Receiver](Receiver.md)
postprocessor named `transfer_in`, as if the following were added to the input file:

!listing
[Postprocessors]
  [transfer_in]
    type = Receiver
  []
[]

The `transfer_in` postprocessor simply receives
the `synchronize` postprocessor from the main application, as shown in
the [MultiAppPostprocessorTransfer](MultiAppPostprocessorTransfer.md)
in the solid input file. We also add two [FieldTransfers](AddFieldTransferAction.md).
The [NekBoundaryFlux](NekBoundaryFlux.md) willtread from an auxiliary variable named `heat_flux` (automatically created by Cardinal) and normalize
according to a postprocessor named `heat_flux_integral` (also automatically created by Cardinal).
The [NekFieldVariable](NekFieldVariable.md) will then read from the field
variable temperature internal to NekRS and write it into an auxiliary variable
(automatically created by Cardinal) named `temperature`.

We specify a number of other postprocessors in order to query the NekRS solution
for each time step.

!listing tutorials/sfr_7pin/nek.i
  block=Postprocessors

Finally, we specify a [Transient](Transient.md)
executioner and [NekTimeStepper](NekTimeStepper.md) in order for
NekRS to choose its time step (subject to any synchronization points specified
by the main application). We also specify an Exodus output file format.

!listing tutorials/sfr_7pin/nek.i
  start=Executioner
  end=Postprocessors

Additional files necessary to set up the NekRS problem are the same files you'd need
to set up a standalone NekRS simulation -

- `sfr_7pin.re2`: NekRS mesh
- `sfr_7pin.par`: High-level settings for the solver, boundary condition mappings to sidesets,
  and the equations to solve
- `sfr_7pin.udf`: User-defined C++ functions for on-line postprocessing and model setup
- `sfr_7pin.oudf`: User-defined [!ac](OCCA) kernels for boundary conditions and source terms

Begin with the `sfr_7pin.par` file.

!listing /tutorials/sfr_7pin/sfr_7pin.par

Boundaries 1 and 2 will receive heat flux from MOOSE, so these two boundaries are set to
flux boundaries, or `f` for the `[TEMPERATURE]` block. Other settings are largely the same.

The assignment of boundary condition values is performed in the
`sfr_7pin.oudf` file, shown below. Note that for boundaries 1 and 2, where we want to receive
heat flux from MOOSE, we set the value of the flux equal to `bc->usrwrk[bc->idM]`, or
the scratch array that is written by [NekRSProblem](NekRSProblem.md).

!listing /tutorials/sfr_7pin/sfr_7pin.oudf language=cpp

Finally, the `sfr_7pin.udf` file contains C++ functions to set up boundary conditions
and perform other post-processing operations. In `UDF_Setup`, we set initial
conditions for velocity, pressure, and temperature. For convenience, we define
local functions like `mass_flowrate()` to be able to set problem
parameters in a single place and use them multiple places (these functions are
*not* NekRS syntax - i.e. we could equivalently have done something like `#define mdot 0.1`).

!listing /tutorials/sfr_7pin/sfr_7pin.udf language=cpp

## Execution and Postprocessing

To run the pseudo-steady model,

```
mpiexec -np 4 cardinal-opt -i solid.i
```

Because we used `synchronization_interval = parent_app`, you will see in the screen output
that the data transfers into NekRS only occur on synchronization points with the
main application - all other time steps will omit the messages about normalizing
heat flux and extracting temperatures from NekRS.

After converting the NekRS output files to a format viewable in Paraview
(see instructions [here](https://nekrsdoc.readthedocs.io/en/latest/detailed_usage.html#visualizing-output-files)),
the simulation results can be displayed. The fluid temperature is shown in [temperature]
along with the mesh lines of the solid phase, while the solid temperature is
shown in [temperature2] along with the lines connecting
[!ac](GLL) points for the fluid phase. The
temperature color scale is the same in both figures.

!media sfr_temperature1.png
  id=temperature
  caption=Fluid temperature computed by NekRS for [!ac](CHT) coupling for a bare 7-pin [!ac](SFR) bundle with solid mesh lines shown in blue.
  style=width:60%;margin-left:auto;margin-right:auto;halign:center

!media sfr_temperature2.png
  id=temperature2
  caption=Solid temperature computed by MOOSE for [!ac](CHT) coupling for a bare 7-pin [!ac](SFR) bundle with fluid mesh lines shown in blue.
  style=width:60%;margin-left:auto;margin-right:auto;halign:center

## Preserving Flux on Individual Sidesets

By default, [NekBoundaryFlux](NekBoundaryFlux.md)
will lump all "receiving" sidesets in NekRS together for the purpose of normalization.
For this example, this means that the pin outer surface flux will not *exactly*
match the pin outer flux from MOOSE (and similarly for the duct inner surface flux),
because these surfaces are lumped together. In this section, we illustrate a more
advanced option to individually preserve flux by sideset.

The input files we will use are the `solid_vpp.i` and `nek_vpp.i` files. These files
are almost identical to the files described in the previous section, so we only
emphasize the differences. First, in the solid model we need to set up individual
postprocessors for the heat flux corresponding to each NekRS boundary.
Then, we need to set up a [VectorOfPostprocessors](VectorOfPostprocessors.md)
to fill a vector with each flux postprocessor. Note that the order
of the postprocessors must match the boundaries they get mapped to in
[NekRSMesh](NekRSMesh.md).

!listing tutorials/sfr_7pin/solid_vpp.i
  start=VectorPostprocessors
  end=MultiApps

Then, we simply need to replace the [MultiAppPostprocessorTransfer](MultiAppPostprocessorTransfer.md)
with a [MultiAppReporterTransfer](MultiAppReporterTransfer.md).

!listing tutorials/sfr_7pin/solid_vpp.i
  block=Transfers

Finally, we just need to set `conserve_flux_by_sideset = true` in [NekBoundaryFlux](NekBoundaryFlux.md)
in the Nek input file.

!listing tutorials/sfr_7pin/nek_vpp.i
  block=Problem

To run the model,

```
mpirun -np 4 cardinal-opt -i solid_vpp.i
```

The physics predictions are nearly identical to those displayed earlier.

## Flux-Flux Coupling Option
  id=fluxflux

In this section, we modify the flux-temperature coupling from [#fluxtemperature]
to instead compute a convective heat flux with NekRS to apply to MOOSE as a Robin-type
boundary condition,

\begin{equation}
\label{eq:hf}
q''=h(T_w-T_\infty)
\end{equation}

where $q''$ is the heat flux that will be imposed on the solid boundaries, $h$
is the convective heat transfer coefficient, $T_w$ is NekRS's wall temperature,
and $T_\infty$ is NekRS's bulk temperature. To use this flux-flux coupling option,
we don't need any changes to Cardinal's source code -- instead, we simply need to
add user objects to the fluid input files to comput $h$ and $T_\infty$.
A [previous tutorial](cht5.md) showed how to compute a heat transfer coefficient
with NekRS; we will use this same foundation now to compute $h$ and then apply this
alternative boundary condition to our solid app.

### Fluid Input Files

The fluid input files are mostly the same as the case with temperature-flux coupling,
so we will only focus on the aspects which are different.

To compute the quantities in Eq. \eqref{eq:hf}, we need to add a few user objects to
our Nek wrapping file. We will do so in the `nek_fluxflux.i` file. For our
heat transfer coefficient, we will compute the heat transfer coefficient (ideally
as a function of space, since it varies with position and this will yield a more
accurate coupling).
First, we need to define how NekRS's mesh will be "chunked" for this calculation.
We will add a [HexagonalSubchannelBin](HexagonalSubchannelBin.md) to define
the volumetric regions of space in the $x-y$ plane and a
[LayeredBin](LayeredBin.md) to define the volumetric regions of space in the
axial direction. Together, these binnings are combined to chunk 3-D space.
Then, we simply add additional user objects to compute the necessary terms
in Eq. \eqref{eq:hf}:

- [NekBinnedSideAverage](NekBinnedSideAverage.md) to compute the average heat flux on the pin surfaces (one unique calculation for each axial layer and for each subchannel). The heat flux from MOOSE is written into the zeroth slot in the `nrs->usrwrk` array, which we indicate as the field we want to average by setting `field = usrwrk00`.
- [NekBinnedSideAverage](NekBinnedSideAverage.md) to compute the average wall temperature on the pin surfaces (one unique calculation for each axial layer and for each subchannel). We indicate temperature by setting `field = temperature`.
- [NekBinnedVolumeAverage](NekBinnedVolumeAverage.md) to compute the average bulk temperature over the subchannel volumes (one unique calculation for each axial layer and for each subchannel). We indicate temperature by setting `field = temperature`.

!listing /tutorials/sfr_7pin/nek_fluxflux.i
  block=UserObjects

Then, we use these user objects to compute a heat transfer coefficient using
[HeatTransferCoefficientAux](HeatTransferCoefficientAux.md). This heat transfer
coefficient will get passed to the solid input file, along the the bulk
temperature, for use in the convective flux boundary condition.

### Solid Input Files

The solid input files are mostly the same as the case with temperature-flux coupling,
so we will only focus on the aspects which are different. First, we will use
a convective heat flux boundary condition on the pin surfaces (for illustration,
we'll leave the Dirichlet temperature condition on the duct inner surface to show
that you can mix-and-match as you please).

!listing /tutorials/sfr_7pin/solid_fluxflux.i
  block=BCs

The variables, `h` and `nek_bulk_temp`, are fetched from transfers with the NekRS
sub-application. The [MultiAppGeneralFieldUserObjectTransfer](MultiAppGeneralFieldUserObjectTransfer.md) object will directly evaluate the userobject which computed
the bulk temperature, at the quadrature points in the solid mesh. The only
other new transfer is passing the variable containing the heat transfer
coefficient, using a [MultiAppGeneralFieldNearestLocationTransfer](MultiAppGeneralFieldNearestLocationTransfer.md).

!listing /tutorials/sfr_7pin/solid_fluxflux.i
  block=Transfers

### Execution and Postprocessing

To run the pseudo-steady model,

```
mpiexec -np 4 cardinal-opt -i solid_fluxflux.i
```

The results are very similar (though not identical) to the flux-temperature coupling
case, because we have to chunk up space in order to compute a heat transfer
coefficient. This discretization can be faintly seen in the flux-flux results
(though can be diminished by simply using a finer spatial chunking for the
heat transfer coefficient).

!media flux_temp.png
  id=flux_temp
  caption=Solid temperature predicted when solving conjugate heat transfer with NekRS using (i) flux-temperature coupling from the earlier part of this tutorial or (ii) flux-flux coupling.
  style=halign:center
