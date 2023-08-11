# Turbulent LES Flow in a Heated Pebble Bed
  id=tutorial1d

In this tutorial, you will learn how to:

- Couple NekRS to MOOSE for [!ac](CHT) for [!ac](LES) in a pebble bed
- Manipulate the NekRS solution on each time step through custom user-defined kernels
- Adjust sidesets in the NekRS mesh prior to usage

To access this tutorial,

```
cd cardinal/tutorials/pebble_67
```

This tutorial also requires you to download some mesh files from Box.
Please download the files from the `pebble_67` folder
[here](https://anl.app.box.com/s/irryqrx97n5vi4jmct1e3roqgmhzic89/folder/141527707499)
and place these files within the same directory structure in
`tutorials/pebble_67`.

!alert! note title=Computing Needs
This tutorial requires [!ac](HPC) resources to run.
!alert-end!

## Geometry and Computational Model

The geometry consists of a small packed bed with 67 spheres each with diameter of $d_p=6$ cm; helium
flows in the space between pebbles. A cut-away through the center of the bed is
shown in [pebble_slice]. The pebbles are packed into a cylinder with diameter of $4.4d_p$.
The pebbles begin around $2.5d_p$ upstream from the cylinder inlet, and the packed region
has a total height of about $5d_p$.

!media pebble_slice.png
  id=pebble_slice
  caption=Cutaway through the domain in a 67-pebble packed bed. Pebbles are shown in red, while the fluid region is shown in gray.
  style=width:30%;margin-left:auto;margin-right:auto

[table1] summarizes the geometry and operating conditions of this model. The Reynolds
number is based on the pebble diameter.

!table id=table1 caption=Geometric and boundary condition specifications for a 67-pebble bed
| Parameter | Value |
| :- | :- |
| Pebble diameter, $d_p$ | 6 cm |
| Inlet temperature | 523 K |
| Outlet pressure | 4 MPa |
| Reynolds number | 1460 |
| Prandtl number | 0.71 |
| Power | 24 kW |

### Heat Conduction Model

The MOOSE heat conduction module is used to solve for [energy conservation in the solid](theory/heat_eqn.md),
with the time derivative neglected in order to more quickly approach steady state.
The solid mesh is shown in [pebble_solid_mesh]; the outer surface of the pebbles is sideset 0.

!media pebble_solid_mesh.png
  id=pebble_solid_mesh
  caption=Mesh for the solid heat conduction model
  style=width:40%;margin-left:auto;margin-right:auto

This mesh is generated using MOOSE mesh generators by building a mesh for a single
sphere and repeating it 67 times, translated to the position of each pebble. The pebble
positions are obtained using off-line discrete element modeling (not discussed here). Note
that while chamfers are accounted for in the fluid mesh, that the 67 pebbles are
not interconnected to one another for simplicity.

!listing /tutorials/pebble_67/moose.i
  block=Mesh

The interior of the pebbles is homogenized. We will also assume a uniform power distribution
in the pebbles.
On the pebble surfaces, a Dirichlet temperature is provided by NekRS.
We will run the solid model first, so we must specify
an initial condition for the wall temperature, which we simply set to the fluid inlet temperature.

### NekRS Model

NekRS solves the [incompressible Navier-Stokes equations](theory/ins.md)
in [non-dimensional form](nondimensional_ns.md). Turbulence is modeled using
[!ac](LES) using a [high-pass filter](les_filter.md).

The NekRS files are:

- `pb67.re2`: NekRS mesh
- `pb67.par`: High-level settings for the solver, boundary condition mappings to sidesets, and the equations to solve
- `pb67.udf`: User-defined C++ functions for on-line postprocessing and model setup
- `pb67.oudf`: User-defined [!ac](OCCA) kernels for boundary conditions, source terms, and other equation manipulations
- `pb67.usr`: Legacy Fortran backend for postprocessing and input modifications

The fluid mesh is shown in [pebble_fluid_mesh] using Paraview's "crinkle clip"
feature to more clearly delineate individual elements. The pebble locations
are shown in red for additional context. This mesh contains 122,284 hexahedral
elements. This mesh is generated using a Voronoi cell approach [!cite](lan).

!media pebble_fluid_mesh.png
  id=pebble_fluid_mesh
  caption=Mesh for the NekRS CFD model; [!ac](GLL) points are not shown
  style=width:60%;margin-left:auto;margin-right:auto

Next, the `.par` file contains problem setup information. This input solves in
non-dimensional form, at a Reynolds number of 1460 and a Peclet number of 1036.
Note the [!ac](LES) filter specification in the `[GENERAL]` block.

!listing /tutorials/pebble_67/pb67.par

In the `.oudf` file, we define boundary conditions.
The inlet boundary is set to a temperature of 0 (a dimensional temperature of
$T_{ref}$), while the fluid-solid interface will receive a heat flux from MOOSE.
The inlet velocity condition is set to $V_z=1$, with a stabilized outflow for pressure.

!listing /tutorials/pebble_67/pb67.oudf language=cpp

We also create two custom kernels which we shall use to manipulate the NekRS solution on
each time step:

- Clip temperature so that it is always within the range of $0\leq T^\dagger\leq 100$,
  where $T^\dagger$ is the non-dimensional temperature. This is useful to limit extreme
  temperatures in under-resolved regions of the mesh. We define this operation in
  a kernel, which we name `clipTemperature`.
- Increase the viscosity and conductivity at the domain outlet (where the mesh is underresolved)
  in order to maintain a stable solution. The properties are modified to ramp
  the viscosity and conductivity between heights of $4.6d_p$ and $5d_p$ in a kernel
  which we name `manipulateOutlet`.

Next, the `.udf` file is used to setup initial conditions and conduct other initialization
aspects in order to JIT-compile our kernels.

!listing /tutorials/pebble_67/pb67.udf language=cpp

Finally, for this tutorial we are also using the legacy Fortran backend to modify
some of our boundary condition sidesets (for demonstration's sake, the mesh may have
been generated without sideset numbers, which NekRS needs). We do this manipulation
in the `pb67.usr` file, in the `usrdat2()` subroutine.

!listing /tutorials/pebble_67/pb67.usr language=fortran

The following lists how to interpret these various Fortran entities:

- `iel` is a loop variable name used to indicate a loop over elements
- `ifc` is a loop variable name used to indicate a loop over the faces on an element
- `cbc(ifc, iel, 1)`: sideset name for velocity
- `cbc(ifc, iel, 2)`: sideset name for temperature
- `bc(5, ifc, iel, 1)`: sideset ID for velocity
- `boundaryID(ifc, iel)`: sideset ID for velocity
- `boundaryIDt(ifc, iel)`: sideset ID for temperature

In other words, this function is essentially (i) changing the names associated with
sidesets and then (ii) using those new names to define IDs for the sidesets.

## CHT Coupling
  id=cht

In this section, MOOSE and NekRS are coupled for [!ac](CHT).

### Solid Input Files

The solid phase is solved with the MOOSE heat conduction module, and is described in
the `moose.i` input. First we set up the mesh using a [SphereMeshGenerator](https://mooseframework.inl.gov/source/meshgenerators/SphereMeshGenerator.html) for each pebble and then repeating it 67 times
at each pebble location.

!listing /tutorials/pebble_67/moose.i
  end=Variables

Next, we define a temperature variable `temp`, and specify the governing equations and
boundary conditions we will apply.

!listing /tutorials/pebble_67/moose.i
  start=Variables
  end=Functions

The MOOSE heat conduction module will receive a wall temperature from NekRS in
the form of an [AuxVariable](https://mooseframework.inl.gov/syntax/AuxVariables/index.html),
so we define a receiver variable for the temperature, as `nek_temp`. The MOOSE
heat conduction module will also send heat flux to NekRS, which we compute as
another [AuxVariable](https://mooseframework.inl.gov/syntax/AuxVariables/index.html)
named `flux`, which we compute with a
[DiffusionFluxAux](https://mooseframework.inl.gov/source/auxkernels/DiffusionFluxAux.html)
auxiliary kernel.

!listing /tutorials/pebble_67/moose.i
  start=AuxVariables
  end=MultiApps

We define a number of postprocessors for querying the solution as well as for
normalizing the heat flux.

!listing /tutorials/pebble_67/moose.i
  block=Postprocessors

Finally, we add a [TransientMultiApp](https://mooseframework.inl.gov/source/multiapps/TransientMultiApp.html)
that will run a MOOSE-wrapped NekRS simulation. Then, we add three different
transfers to/from NekRS:

- [MultiAppNearestNodeTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppNearestNodeTransfer.html)
  to send the heat flux from MOOSE to NekRS
- [MultiAppNearestNodeTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppNearestNodeTransfer.html)
  to send temperature from NekRS to MOOSE
- [MultiAppPostprocessorTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppPostprocessorTransfer.html)
  to normalize the heat flux sent to NekRS

!listing /tutorials/pebble_67/moose.i
  start=MultiApps
  end=Postprocessors

#### Fluid Input Files
  id=nek

The Nek wrapping is described in the `nek.i` input file.
We first define
a few file-local variables, and then build a mesh mirror with a
[NekRSMesh](https://cardinal.cels.anl.gov/source/mesh/NekRSMesh.html). By setting
`boundary = '4'`, we indicate that boundary 4 will be coupled via [!ac](CHT).

!listing /tutorials/pebble_67/nek.i
  end=Problem

Next, we define additional parameters to describe how NekRS interacts with MOOSE
with the [NekRSProblem](https://cardinal.cels.anl.gov/source/problems/NekRSProblem.html).
The NekRS input files are in nondimensional form, so we must indicate all the characteristic
scales so that data transfers with a dimensional MOOSE application are performed
correctly.

!listing /tutorials/pebble_67/nek.i
  block=Problem

For time stepping, we will allow NekRS to control its own time stepping
by using the [NekTimeStepper](https://cardinal.cels.anl.gov/source/timesteppers/NekTimeStepper.html).

!listing /tutorials/pebble_67/nek.i
  start=Executioner
  end=Postprocessors

Then, we define a few postprocessors to use for querying the solution.

!listing /tutorials/pebble_67/nek.i
  block=Postprocessors

## Execution and Postprocessing

To run the coupled NekRS-MOOSE calculation,

```
mpiexec -np 500 cardinal-opt -i moose.i
```

This will run with 500 [!ac](MPI) processes.

When the simulation has finished, you will have created a number of different
output files:

- `moose_out.e`, an Exodus file with the MOOSE solution for the NekRS-MOOSE calculation
- `moose_out_nek0.e`, an Exodus file with the NekRS solution that was ultimately transferred
  in/out of MOOSE
- `pb670.f<n>`, a (custom format) NekRS output file with the NekRS solution; to convert
  to a format viewable in Paraview, consult the [NekRS documentation](https://nekrsdoc.readthedocs.io/en/latest/detailed_usage.html#visualizing-output-files)

[pb67_solution] shows the instantaneous velocity (left) and temperature (right) predicted
by NekRS, in non-dimensional form. This solution is visualized from the NekRS native output files.

!media pb67_solution.png
  id=pb67_solution
  caption=Instantaneous velocity (left) and temperature (right) predicted by NekRS, in non-dimensional form.
  style=width:50%;margin-left:auto;margin-right:auto
