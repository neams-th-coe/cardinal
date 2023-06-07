# DAGMC pincell

In this tutorial, you will learn how to:

- Couple CAD Monte Carlo models via temperature and heat source feedback to MOOSE

To access this tutorial,

```
cd cardinal/tutorials/dagmc
```

!alert! note title=Computing Needs
No special computing needs are required for this tutorial.
For testing purposes, you may choose to decrease the number of particles to
solve faster.

To run this tutorial,
you need to have built Cardinal with DAGMC support enabled, by setting
`export ENABLE_DAGMC=true`.
!alert-end!

## Geometry and Computational Models

This model consists of a pincell, taken directly from an
[OpenMC DAGMC tutorial](https://github.com/openmc-dev/openmc-notebooks/blob/main/cad-based-geometry.ipynb).
[!ac](DAGMC) is a package for Monte Carlo transport on [!ac](CAD) geometry.
We will not go into detail on how the DAGMC model was generated, but instead
refer you to the [DAGMC documentation](https://svalinn.github.io/DAGMC/usersguide/index.html).

The geometry consists of a U-235 cylinder enclosed in an annulus of water. The height is 40
cm, and the outer diameter of the fuel cylinder is 18 cm.
The total power is set to 1000 W. An image of the geometry is shown on the $x$-$y$ plane in
[dagmc_geom].

!media dagmc_pincell.png
  id=dagmc_geom
  caption=OpenMC DAGMC geometry colored by material, on the $x$-$y$ plane.
  style=width:30%;margin-left:auto;margin-right:auto

[dagmc_cells] shows the same DAGMC model, colored instead by cell. The two green regions
are both U-235, while the purple is water. There is no particular reason why the model is
built this way, since we are just fetching it from an existing OpenMC tutorial.

!media dagmc_pincell_cell.png
  id=dagmc_cells
  caption=OpenMC DAGMC geometry colored by cell, on the $x$-$y$ plane.
  style=width:30%;margin-left:auto;margin-right:auto

### MOOSE Heat Conduction Model

!include steady_hc.md

[MeshGenerators](https://mooseframework.inl.gov/syntax/Mesh/index.html) are used to construct
the solid mesh. [solid_mesh2] shows the solid mesh with block IDs and sidesets. We will
not model any heat transfer in the water region, for simplicity.
Different block IDs are used for the hexahedral and prism elements
in the pellet region because libMesh does not allow different element types
to exist on the same block ID. To generate the mesh,

!listing
cardinal-opt -i mesh.i --mesh-only

!media pincell_solid_mesh2.png
  id=solid_mesh2
  caption=Mesh for the solid portions of a pincell
  style=width:60%;margin-left:auto;margin-right:auto

The surface of the pincell is simply set to a constant value, $T_s=500$.
Because heat transfer and fluid flow in the borated water is not modeled in this example,
The top and bottom of the solid pincell
are assumed insulated.
In this example, the MOOSE heat conduction module will run first. The initial
solid temperature is 500&deg;C and the initial power is zero.

### OpenMC Model

The OpenMC model is built using DAGMC. Particles move through space with surface-to-surface
tracking between triangle surface meshes. Cells are the regions of space enclosed by
these surfaces. After building the DAGMC model with Cubit, we set up the OpenMC input files
using the Python [!ac](API). First, we define
materials, then fetch the geometry from the DAGMC geometry file (`dagmc.h5m`). Then, we
set up the settings, for numbers of batches and particles, and how we would like to
have temperature feedback applied to OpenMC.

!listing /tutorials/dagmc/make_model.py

To generate the XML files needed to run OpenMC, you can run the following:

```
python make_model.py
```

or simply use the XML files checked in to the `tutorials/dagmc` directory.

## Multiphysics Coupling
  id=coupling

In this section, OpenMC and MOOSE are coupled for heat source and temperature feedback
for a DAGMC pincell.
The following sub-sections describe these files.

### Solid Input Files

The solid phase is solved with the MOOSE heat conduction module, and is
described in the `solid.i` input. The solid mesh is created using mesh generators
in the `mesh.i` input:

!listing /tutorials/dagmc/mesh.i

We generate the mesh by running `cardinal-opt -i mesh.i --mesh-only` to create the
`mesh_in.e` file, which we then use in the solid input file.

!listing /tutorials/dagmc/solid.i
  end=Variables

The heat conduction module will solve for temperature, with the heat equation.
The variables, kernels, and boundary conditions are shown below.

!listing /tutorials/dagmc/solid.i
  start=Variables
  end=Executioner

The [Transfer](https://mooseframework.inl.gov/syntax/Transfers/index.html)
system in MOOSE is used to communicate variables across applications; a
heat source will be computed by OpenMC and applied as a source term in MOOSE.
In the opposite direction, MOOSE will compute a temperature that will be
applied to the OpenMC geometry. The `heat_source` auxiliary variable will simply
receive the heat source from OpenMC.
The [MultiApps](https://mooseframework.inl.gov/syntax/MultiApps/index.html)
and [Transfers](https://mooseframework.inl.gov/syntax/Transfers/index.html)
blocks describe the interaction between Cardinal and MOOSE. The MOOSE
heat conduction application is run as the main application, with
OpenMC run as the sub-application. We specify that MOOSE will run
first on each time step.

Two transfers are required to couple OpenMC and MOOSE for heat source and
temperature feedback. The first is a transfer of heat source from Cardinal
to MOOSE. The second is transfer of temperature from MOOSE to Cardinal.

!listing /tutorials/dagmc/solid.i
  start=MultiApps
  end=Postprocessors

For the heat source transfer from OpenMC, we ensure conservation by
requiring that the integral of heat source computed by OpenMC
(in the `heat_source` postprocessor) matches the integral of the heat
source received by MOOSE (in the `source_integral` postprocessor).
We also add a postprocessor to evaluate the maximum solid temperature.

!listing /tutorials/dagmc/solid.i
  block=Postprocessors

Because we did not specify sub-cycling
in the `[MultiApps]` block, this means that OpenMC will run for exactly the
same number of time steps (but the actual time step size used by the OpenMC
wrapping is of no consequence because OpenMC is run in $k$-eigenvalue mode).
By setting a fixed number of time steps, this example
will simply run a fixed number of Picard iterations.

!listing /tutorials/dagmc/solid.i
  start=Executioner
  end=MultiApps

### Neutronics Input Files

The neutronics physics is solved over the entire domain using OpenMC.
The OpenMC wrapping is described in the `openmc.i` input file.
We begin by defining a mesh on which OpenMC
will receive temperature from the coupled MOOSE application, and on which OpenMC
will write the fission heat source. In this example, we use exactly the same solid
mesh as the coupled MOOSE application; this is not a requirement, and is relaxed
in other tutorials.

!listing /tutorials/dagmc/openmc.i
  end=AuxVariables

Next, the [Problem](https://mooseframework.inl.gov/syntax/Problem/index.html)
block describes all objects necessary for the actual physics solve. To replace
MOOSE finite element calculations with OpenMC particle transport calculations,
the [OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md) class
is used.

!listing /tutorials/dagmc/openmc.i
  block=Problem

For this example, we specify the total fission power by which to normalize OpenMC's
tally results (because OpenMC's tally results are in units of eV/source particle).
Next, we indicate which blocks in the `[Mesh]` should be considered
as "solid" (and therefore send temperatures into OpenMC) with `solid_blocks`.
Here, we specify temperature feedback for the pellet (blocks 2 and 3).
During the initialization, [OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md)
will automatically map from MOOSE elements to OpenMC cells, and store which MOOSE elements
are "solid." Then when temperature is sent into OpenMC, that mapping is used to compute
a volume-average temperature to apply to each OpenMC cell.

This example uses mesh tallies, as indicated by the
`tally_type`.
[OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md) will then
automatically add the necessary tallies.
Finally, we specify the level in the geometry on which the solid cells
exist. Because we don't have any lattices or filled universes in our OpenMC model,
the solid cell level is zero.

!alert note
At this point, it is important to remind ourselves of the problem setup. OpenMC
is tracking particles with surface-to-surface tracking for a tetrahedral mesh
(the `dagmc.h5m` geometry). However, the `[Mesh]` block is just overlaid on
the problem, and we tally on this separate mesh. The two meshes (the tally mesh
in the `[Mesh]` block vs. the DAGMC mesh) are completely unrelated to one another.

Next, we add a series of auxiliary variables for solution visualization
(these are not requried for coupling). To help with understanding
how the OpenMC model maps to the mesh in the `[Mesh]` block, we add auxiliary
variables to visualize OpenMC's cell ID ([CellIDAux](/auxkernels/CellIDAux.md))
and cell temperature ([CellTemperatureAux](/auxkernels/CellTemperatureAux.md)).

!listing /tutorials/dagmc/openmc.i
  start=AuxVariables
  end=Problem

Next, we specify an executioner and output settings. Even though OpenMC technically
performs a criticality calculation (with no time dependence), we use the transient
executioner so that *if* we wanted to run OpenMC more times than the coupled
main application via subcycling, we would have a way to control that.

!listing /tutorials/dagmc/openmc.i
  start=Executioner
  end=Postprocessors

Finally, we add a postprocessor to evaluate the total heat source computed by OpenMC
and query other parts of the solution.

!listing /tutorials/dagmc/openmc.i
  block=Postprocessors

## Execution and Postprocessing

To run the coupled calculation,

```
mpiexec -np 2 cardinal-opt -i solid.i --n-threads=2
```

This will run both MOOSE and OpenMC with 2 MPI processes and 2 OpenMP threads per rank.
To run the simulation faster, you can increase the parallel processes/threads, or
simply decrease the number of particles used in OpenMC.
When the simulation has completed, you will have created a number of different output files:

- `solid_out.e`, an Exodus output with the solid mesh and solution
- `solid_out_openmc0.e`, an Exodus output with the OpenMC solution and the data
  that was ultimately transferred in/out of OpenMC

[dagmc_heat_source] shows the heat source computed by OpenMC (units of W/cm$^3$)
and mapped to the MOOSE mesh, the solid temperature computed by MOOSE, and the
temperature imposed in OpenMC. The two images showing temperature share a color scale,
and are depicted as a slice through the pellet. Because the DAGMC model only has two cells to
represent the fuel region, the temperature imposed in OpenMC is a volume average
over the elements corresponding to each of those two cells.
If you want to resolve the solid temperature
with more detail in the OpenMC model, simply add OpenMC cells where finer feedback
is desired - *or*, you can adaptively re-generate the OpenMC cells using
the [MoabSkinner](https://cardinal.cels.anl.gov/source/userobjects/MoabSkinner.html).

!media dagmc_heat_source.png
  id=dagmc_heat_source
  caption=Heat source (W/cm$^3$) computed by OpenMC (left); MOOSE solid temperature (middle); OpenMC cell temperature (right)
