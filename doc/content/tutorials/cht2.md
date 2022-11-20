# Conjugate Heat Transfer for Laminar Pin Bundle Flow

In this tutorial, you will learn how to:

- Couple NekRS with MOOSE for [!ac](CHT) for a 7-pin bundle
- Solve both NekRS and MOOSE in dimensional form
- Control how flux normalization is performed in NekRS (by either lumping all sidesets together,
  or preserving for each sideset individually)
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

!alert! note title=Computing Needs
No special computing needs are required for this tutorial.
!alert-end!

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
| Wire axial lead length | 0.2032 m |
| Core power | 250 MWth |
| Core inlet temperature | 350&deg;C |

Heat is produced in the pellet region and transfers by radiation and conduction across
the pellet-clad gap to the cladding. Sodium flows around the pins to remove the fission heat.
The power in the solid phase is set equal to

\begin{equation}
\dot{q}_s=\frac{250 \text{ MWth}}{n_b n_p H \pi R_p^2}
\end{equation}

where $n_b=54$ is the number of power-producing bundles, $n_p=217$ is the (nominal) number of
fuel pins per bundle, $H=0.8$ is the height of the active region, and $R_p$ is the outer
radius of the fuel pellet.

### Heat Conduction Model

The MOOSE heat conduction module is used to solve for [energy conservation in the solid](theory/heat_eqn.md).
The outer surface of the duct and the tops and bottoms of the
pins and ducts are assumed insulated. At fluid-solid interfaces, the solid temperature
is imposed as a Dirichlet condition, where NekRS computes the surface temperature.

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
$r_{th}$ is the conduction resistance. For cylindrical geometries, the conduction
resistance is given as

\begin{equation}
\label{eq:2}
r_{th}=\frac{ln{\left(\frac{r_2}{r_1}\right)}}{2\pi L k}
\end{equation}

where $r_2>r_1$ are the radial coordinates associated with the outer and inner radii
of the cylindrical annulus, $L$ is the height of the annulus, and $k$ is the
thermal conductivity of the annulus material.

Both the solid temperature and the surface temperature boundary condition (that will
be provided by NekRS) are set to an initial condition of 500 K.

### NekRS Model

NekRS is used to solve the [incompressible Navier-Stokes equations](theory/ins.md).
At the inlet, the fluid temperature is taken as 355&deg;C. The inlet velocity
is selected such that the mass flowrate is 0.1 kg/s, which is low enough that the flow is
laminar and a turbulence model is not required for this pedagogical example.

At the outlet, a zero pressure is imposed and an outflow condition is applied for
the energy conservation equation. On all solid-fluid interfaces, the velocity is set
to the no-slip condition and a heat flux is imposed in the energy conservation equation,
where MOOSE computes the surface heat flux.

The initial pressure is set to zero. Both the velocity and temperature
are set to uniform initial conditions that match the inlet conditions.

## Meshing
  id=meshing

A combination of Cubit [!cite](cubit) and MOOSE
[MeshGenerators](https://mooseframework.inl.gov/syntax/Mesh/index.html)
are used to generate the meshes for the fluid and solid phases.

### Solid Mesh

The solid mesh is shown in [solid_mesh]; because this mesh is generated using
a combination of Cubit-generated meshes (for the duct) and
MOOSE [MeshGenerators](https://mooseframework.inl.gov/syntax/Mesh/) (for everything else), so you can
view the mesh by running the simulation (and viewing the mesh
on which the results are displayed), or simply by running the solid input file in
mesh generation mode:

!listing
cardinal-opt -i solid.i --mesh-only

The boundary names are illustrated towards
the right by showing only the highlighted surface to which each boundary corresponds.
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
surfaces, as well as on the outside of the duct, we don't need to define sidesets (because
the "do-nothing" boundary condition in the finite element method is a zero-flux condition).

### Fluid Mesh

The complete fluid mesh is shown in [fluid_mesh]; the boundary names are illustrated towards the right
by showing only the highlighted surface to which each boundary corresponds.

!media sfr_fluid_mesh.png
  id=fluid_mesh
  caption=Mesh for the fluid portions of the 7-pin bare [!ac](SFR) bundle
  style=halign:center

Note that the meshes in [solid_mesh] and [fluid_mesh]
do not need to share nodes between the fluid and solid
boundaries through which [!ac](CHT) coupling will be performed - interpolations
using MOOSE's [Transfers](https://mooseframework.inl.gov/syntax/Transfers/index.html)
handle any differences in the mesh.

## CHT Coupling

In this section, NekRS and MOOSE are coupled for [!ac](CHT) between the sodium
coolant and the solid pincells and duct.

### Solid Input Files

The solid phase is solved with the MOOSE heat conduction module, and is described
in the `solid.i` input file. At the top of this file, various problem parameters
are defined as file-local variables to help with setting up the uniform heat
source in the fuel.

!listing tutorials/sfr_7pin/solid.i
  end=Mesh

Next, the solid mesh is specified through a combination of
[MeshGenerators](https://mooseframework.inl.gov/syntax/Mesh/index.html) and Exodus
mesh files. First, we make a circular annulus that is a 2-D representation of the clad.
We then extrude it into a 3-D shape, and change the numeric values associated with some
sidesets so that we have unique sidesets for all boundaries of interest (we are going
to use more mesh generators). A similar process is then performed to create the mesh
for the pincell. We then combine the clad and pincell together and translate this combined
mesh to the locations of the seven pins. Finally, we combine the seven pins with a
pre-generated duct mesh.

!listing tutorials/sfr_7pin/solid.i
  block=Mesh

The heat conduction module will solve for temperature, which is defined as a nonlinear
variable.

!listing tutorials/sfr_7pin/solid.i
  block=Variables

The [Transfer](https://mooseframework.inl.gov/syntax/Transfers/index.html)
system in MOOSE is used to communicate auxiliary variables across applications; a boundary
heat flux will be computed by MOOSE and applied as a boundary condition in NekRS. In the
opposite direction, NekRS will compute a surface temperature that will be applied as a
boundary condition in MOOSE. Therefore, we define auxiliary variables to hold the flux
computation by MOOSE (`avg_flux`) and the surface temperature received from NekRS
(`nek_temp`).

!listing tutorials/sfr_7pin/solid.i
  block=AuxVariables

Next, the governing equation solved by MOOSE is specified with the `Kernels` block as
the [HeatConduction](https://mooseframework.inl.gov/source/kernels/HeatConduction.html)
 kernel with a uniform volumetric heat source in the pellets with the
[BodyForce](https://mooseframework.inl.gov/source/kernels/BodyForce.html) kernel.
Notice how we can do math with the file-local variables that were defined at the
top of the file, with `${fparse <math statement>}` syntax.

!listing tutorials/sfr_7pin/solid.i
  block=Kernels

For computing the heat flux on the boundaries coupled to NekRS (the clad outer surface
and the duct inner surface), the [DiffusionFluxAux](https://mooseframework.inl.gov/source/auxkernels/DiffusionFluxAux.html)
auxiliary kernel is used.

!listing tutorials/sfr_7pin/solid.i
  block=AuxKernels

The [HeatConductionMaterial](https://mooseframework.inl.gov/source/materials/HeatConductionMaterial.html)
is then used to specify functional-forms for the thermal conductivity
of the pellet, clad, and duct. For the [HeatConductionMaterial](https://mooseframework.inl.gov/source/materials/HeatConductionMaterial.html), you can use
`t` in [ParsedFunctions](https://mooseframework.inl.gov/source/functions/MooseParsedFunction.html)
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

Next, the [MultiApps](https://mooseframework.inl.gov/syntax/MultiApps/index.html)
 and [Transfers](https://mooseframework.inl.gov/syntax/Transfers/index.html)
blocks describe the interaction between Cardinal
and MOOSE. The MOOSE heat conduction module is here run as the main application, with
the NekRS wrapping run as the sub-application.
Three transfers are required to couple Cardinal and MOOSE; the first is a transfer
of surface temperature from Cardinal (specifically, the NekRS wrapping) to MOOSE.
The second is a transfer of heat flux from
MOOSE to Cardinal. And the third is a transfer of the total integrated heat flux from MOOSE
to Cardinal (computed as a postprocessor), which is then used internally by NekRS to re-normalize the heat flux (after
interpolation onto its [!ac](GLL) points).

!listing tutorials/sfr_7pin/solid.i
  start=MultiApps
  end=AuxVariables

In addition to these three transfers necessary to couple NekRS with MOOSE,
there is a fourth transfer - `synchronize_in`, which transfers the `synchronize`
postprocessor to NekRS. The `synchronize` postprocessor
is simply a [Receiver](https://mooseframework.inl.gov/source/postprocessors/Receiver.html)
postprocessor that is set to a value of 1. No applications will transfer anything
*in* to `synchronize`, so the value of this postprocessor remains always fixed
at 1.

!listing tutorials/sfr_7pin/solid.i
  block=synchronize

To understand the purpose of this transfer, we need to describe in more
detail the data transfers that occur when sub-cycling. Please note that this is an
advanced feature added for very large runs to squeeze out as much performance as possible -
understanding this feature is not necessary for using Cardinal.

Consider the case where the main application has a time step of 1,
but NekRS has a time step of 0.2. After the solution of the main application, the heat flux
transfer into NekRS consists of several steps:

1. Transfer from `avg_flux` in the main application to the `avg_flux` receiver variable in
   the MOOSE-wrapped NekRS app.
   This is the transfer that happens in the `Transfers` block.
2. Once the heat flux is available in the `avg_flux` variable in the `nek.i` input, transfer
   that heat flux into NekRS on the host (i.e. the CPU) by interpolating from the
   [NekRSMesh](/mesh/NekRSMesh.md) to the [!ac](GLL) points.
3. Once the heat flux has been normalized on the host, it is then copied from the host to the device (i.e. the parallel backend,
   which will be either a CPU or GPU).

If NekRS is run with a much smaller time step than the main application,
steps 2 and 3 can be omitted to save on the interpolation from the [NekRSMesh](/mesh/NekRSMesh.md)
to NekRS's [!ac](GLL) points *and* on the copy from the host to the device.
Unfortunately, MOOSE's design does not propagate any information about the
coupling hierarchy from a main application to its sub-application(s). So, the use
of this postprocessor (plus some settings in [NekRSProblem](/problems/NekRSProblem.md), to be
discussed shortly) can be used to only perform steps 2 and 3 *if the time stepping
is on the synchronization points* between NekRS and MOOSE. In other words,
if NekRS runs with a time step 100 times smaller than a main application,
this feature of [NekRSProblem](/problems/NekRSProblem.md) can be used to reduce the mesh interpolation
and host-to-device copying by a factor of 100. By transferring this "dummy"
postprocessor to the NekRS wrapping, a signal of the synchronization points,
and therefore the need to actually send data into NekRS, will be available.

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

Next, [NekRSProblem](/problems/NekRSProblem.md) is used to describe all aspects of the
NekRS wrapping.

!listing tutorials/sfr_7pin/nek.i
  block=Problem

We use `synchronization_interval = parent_app` to
indicate that we want to limit the data interpolations from NekRS's [!ac](GLL) points
to the mesh mirror, and the copies between CPU/GPU, to only occur when new coupling data is
to be sent/received from the coupled MOOSE application. When this option is used,
[NekRSProblem](/problems/NekRSProblem.md) automatically adds a
[Receiver](https://mooseframework.inl.gov/source/postprocessors/Receiver.html)
postprocessor named `transfer_in`, as if the following were added to the input file:

!listing
[Postprocessors]
  [transfer_in]
    type = Receiver
  []
[]

The `transfer_in` postprocessor simply receives
the `synchronize` postprocessor from the main application, as shown in
the [MultiAppPostprocessorTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppPostprocessorTransfer.html)
in the solid input file. The value of this postprocessor is then used to
infer whether new data is available/requested from the coupled MOOSE application.

We specify a number of other postprocessors in order to query the NekRS solution
for each time step. Note that the
`flux_integral` receiver postprocessor that the main application sends the flux
integral to does not appear in the input - this postprocessor, like `temp` and
`avg_flux` auxiliary variables, are added automatically by [NekRSProblem](/problems/NekRSProblem.md).

!listing tutorials/sfr_7pin/nek.i
  block=Postprocessors

Finally, we specify a [Transient](https://mooseframework.inl.gov/source/executioners/Transient.html)
executioner and [NekTimeStepper](/timesteppers/NekTimeStepper.md) in order for
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

This input differs from the `.par` file in [Tutorial 2A](cht1.md) in
that the input is in dimensional form.
Boundaries 1 and 2 will receive heat flux from MOOSE, so these two boundaries are set to
flux boundaries, or `f` for the `[TEMPERATURE]` block. Other settings are largely the same.

The assignment of boundary condition values is performed in the
`sfr_7pin.oudf` file, shown below. Note that for boundaries 1 and 2, where we want to receive
heat flux from MOOSE, we set the value of the flux equal to `bc->wrk[bc->idM]`, or
the scratch array that is written by [NekRSProblem](/problems/NekRSProblem.md).

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

By using the `synchronization_interval = parent_app` feature, you will see in the screen output
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

In the first part of this tutorial, we used default settings for how to renormalize
flux sent from MOOSE to NekRS in order to conserve power. By default,
[NekRSProblem](https://cardinal.cels.anl.gov/source/problems/NekRSProblem.html)
will lump all "receiving" sidesets in NekRS together for the purpose of normalization.
For this example, this means that the pin outer surface flux will not *exactly*
match the pin outer flux from MOOSE (and similarly for the duct inner surface flux),
because these surfaces are lumped together. In this section, we illustrate a more
advanced option to individually preserve flux by sideset.

The input files we will use are the `solid_vpp.i` and `nek_vpp.i` files. These files
are almost identical to the files described in the previous section, so we only
emphasize the differences. First, in the solid model we need to set up individual
postprocessors for the heat flux corresponding to each NekRS boundary.
Then, we need to set up a [VectorOfPostprocessors](https://mooseframework.inl.gov/source/vectorpostprocessors/VectorOfPostprocessors.html)
to basically fill a vector with each flux postprocessor. Note that the order
of the postprocessors must match the boundaries they get mapped to in
[NekRSMesh](https://cardinal.cels.anl.gov/source/mesh/NekRSMesh.html).

!listing tutorials/sfr_7pin/solid_vpp.i
  start=VectorPostprocessors
  end=MultiApps

Then, we simply need to replace the [MultiAppPostprocessorTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppPostprocessorTransfer.html)
with a [MultiAppReporterTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppReporterTransfer.html).

!listing tutorials/sfr_7pin/solid_vpp.i
  block=Transfers

Finally, we just need to set `conserve_flux_by_sideset = true` in `NekRSProblem`
in the Nek input file.

!listing tutorials/sfr_7pin/nek_vpp.i
  block=Problem

To run the model,

```
mpirun -np 4 cardinal-opt -i solid_vpp.i
```

The physics predictions are nearly identical to those displayed earlier.

!bibtex bibliography
