# DAGMC Tokamak

In this tutorial, you will learn how to:

- Export DAGMC models from Coreform Cubit
- Couple CAD Monte Carlo models via temperature and heat source feedback to MOOSE
- Use on-the-fly geometry regeneration to resolve temperature/density feedback in OpenMC

To access this tutorial,

```
cd cardinal/tutorials/tokamak
```

This tutorial also requires you to download
some mesh files from Box. Please download the files
from the `tokamak` folder [here](https://anl.app.box.com/folder/276545912801)
and place these files within `tutorials/tokamak`.

!alert! note title=DAGMC build
To run this tutorial, you need to have built Cardinal with DAGMC support enabled, by setting
`export ENABLE_DAGMC=true`.
!alert-end!

## Geometry and Computational Models

This model consists of a 360-degree model of a tokamak, built using capabilities in [Paramak](https://paramak.readthedocs.io/en/main/). To further simplify the details in order to be suitable for a tutorial, the domain only consists of a tungsten first wall, homogenized multiplier and breeder layers, enclosed in a vacuum vessel. A simplified divertor component is also included.
An image of an azimuthal slice of the CAD geometry is shown in
[dagmc_tokamak].

!media dagmc_tokamak.png
  id=dagmc_tokamak
  caption=OpenMC DAGMC geometry colored by material; the names shown on the right correspond to the subdomain names
  style=width:40%;margin-left:auto;margin-right:auto

From Paramak, a STEP file is generated which can then be imported into Coreform Cubit. From within Coreform Cubit, we write a journal file (which can be "played" within the GUI) in order to assign materials, subdomain names, and boundary conditions. One notable difference from how we use DAGMC within Cardinal is the generation of two different meshes:

- A mesh on which to solve the heat conduction problem, as well as to read/write data coupled in/out of OpenMC (`tokamak.e`). This mesh is generated in units of meters.
- A triangulated surface mesh for transporting particles within OpenMC. This mesh is generated in units of centimeters.

!listing tutorials/tokamak/tokamak.jou language=python

After running the journal file, click "File -> Export -> DAGMC" in order to generate the `tokamak.h5m` file. Alternatively, you can download the mesh file and corresponding `.h5m` file from Box.

Paramak can provide a DAGMC geometry file to run OpenMC directly on. The reason only a STEP file was generated from Paramak, was to edit the CAD geometry to add multilayers for (tungsten first wall, multiplier, and breeder) and prepare the geometry to generate a volumetric mesh. The DAGMC h5m file was then exported from Cubit to match the final geometry which the volumetric mesh was generated from such that OpenMC cells are mapped correctly to MOOSE mesh elements.
### MOOSE Heat Conduction Model

!include steady_hc.md

[solid_mesh3] shows a wedge of the 360-degree solid mesh.

!media tokamak_solid_mesh.png
  id=solid_mesh3
  caption=Mesh for the heat conduction problem; colors correspond to different subdomains
  style=width:40%;margin-left:auto;margin-right:auto

For simplicity, all regions will be modeled as purely conducting (no advection). This is a significant
simplification over a realistic fusion device, and therefore the results obtained in this tutorial
should not be taken as representative of a realistic device. However, for the purposes of a tutorial.
there is minimal differences to establishing multiphysics feedback to OpenMC when adding
a fluid solver. Other tutorials which apply density feedback, in addition to temperature feedback,
can be found [here](https://cardinal.cels.anl.gov/tutorials/openmc_fluid.html) and [here](https://cardinal.cels.anl.gov/tutorials/coupled.html).

To approximate some cooling in the breeder and divertor, we apply a uniform heat sink kernel. The magnitude of this heat sink is automatically computed in-line to obtain an approximate energy balance, by evaluating the difference in the heat deposition and heat flux on the vacuum vessel outer wall, divided by the volume in which the cooling is to be applied.

The boundary conditions applied to the heat conduction model are also highly simplified. On the exterior of the vacuum vessel, the temperature is set to a Dirichlet condition of 800 K. On all other sidesets, the boundary is assumed insulated.

### OpenMC Model

The OpenMC model is built using DAGMC. Particles move through space with surface-to-surface
tracking between triangle surface meshes. Cells are the regions of space enclosed by
these surfaces. After building the DAGMC model with Cubit, we set up the OpenMC input files
using the Python [!ac](API). First, we define
materials, then fetch the geometry from the DAGMC geometry file (`tokamak.h5m`). Then, we
set up the settings, for numbers of batches and particles, and how we would like to
have temperature feedback applied to OpenMC. The neutron source is set to a simple ring source, though more realistic fusion sources can be obtained accounting for the plasma parameters, such as some approximate pre-built sources [here](https://github.com/fusion-energy/openmc-plasma-source).

!listing /tutorials/tokamak/model.py

To generate the XML files needed to run OpenMC, you can run the following:

```
python model.py
```

or simply use the XML files checked in to the `tutorials/tokamak` directory.

## Multiphysics Coupling
  id=coupling

In this section, OpenMC and MOOSE are coupled for heat source and temperature feedback
for a tokamak.
The following sub-sections describe these files.

### Heat Conduction Files

The thermal physics is solved with the MOOSE heat transfer module, and is
described in the `solid.i` input. The solid mesh is loaded from a file.
Our Cubit mesh did not have any sidesets yet, so we add a sideset around the
outside of the `vacuum_vessel` subdomain in order to later apply a
boundary condition.

!listing /tutorials/tokamak/solid.i
  block=Mesh

The heat transfer module will solve for temperature, with the heat equation.
The variables, kernels, thermal conductivities, and boundary conditions are shown below.

!listing /tutorials/tokamak/solid.i
  start=Variables
  end=Executioner

The [MultiApps](https://mooseframework.inl.gov/syntax/MultiApps/index.html)
and [Transfers](https://mooseframework.inl.gov/syntax/Transfers/index.html)
blocks describe the interaction between Cardinal and MOOSE. The MOOSE
heat conduction application is run as the main application, with
OpenMC run as the sub-application. We specify that MOOSE will run
first on each time step.

Two transfers are required to couple OpenMC and MOOSE for heat source and
temperature feedback. The first is a transfer of heat source from Cardinal
to MOOSE. The second is transfer of temperature from MOOSE to Cardinal.

!listing /tutorials/tokamak/solid.i
  start=MultiApps
  end=Postprocessors

For the heat source transfer from OpenMC, we ensure conservation by
requiring that the integral of heat source computed by OpenMC
(in the `heating` postprocessor) matches the integral of the heat
source received by MOOSE (in the `source_integral` postprocessor).

Additional postprocessors are added to compute several integrals in-line
in order to apply the heat sink term to approximate an energy balance.
The total heat which must be removed by the heat sink added in the
breeder and divertor will be equal to the magnitude of the nuclear heating,
minus any heat flux from the vacuum vessel surface. This quantity is computed
in the `to_be_removed` postprocessor. This power, divided by the volume
of the breeder and divertor (the `volume` postprocessor), is then applied
to the `heat_removed_density` auxvariable.

!listing /tutorials/tokamak/solid.i
  block=Postprocessors

Because we did not specify sub-cycling
in the `[MultiApps]` block, this means that OpenMC will run for exactly the
same number of steps (i.e., Picard iterations).

!listing /tutorials/tokamak/solid.i
  start=Executioner
  end=MultiApps

### Neutronics Input Files

The neutronics physics is solved over the entire domain using OpenMC.
The OpenMC wrapping is described in the `openmc.i` input file.
We begin by defining a mesh on which OpenMC
will receive temperature from the coupled MOOSE application, and on which OpenMC
will write the nuclear heating.

!listing /tutorials/tokamak/openmc.i
  end=AuxVariables

Next, the [Problem](https://mooseframework.inl.gov/syntax/Problem/index.html)
block describes all objects necessary for the actual physics solve. To replace
MOOSE finite element calculations with OpenMC particle transport calculations,
the [OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md) class
is used.

!listing /tutorials/tokamak/openmc.i
  block=Problem

For this example, we specify the total neutron source rate (neutrons/s) by which to normalize OpenMC's
tally results (because OpenMC's heating tally results are in units of eV/source particle).
Next, we indicate which blocks in the `[Mesh]` should be considered
for temperature feedback using `temperature_blocks`.
Here, we specify temperature feedback for all blocks in the mesh.
During the initialization, [OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md)
will automatically map from MOOSE elements to OpenMC cells, and store which MOOSE elements
are providing temperature feedback. Then when temperature is sent into OpenMC, that mapping is used to compute
a volume-average temperature to apply to each OpenMC cell.

This example uses mesh tallies, as indicated by the
`tally_type`.
[OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md) will then
automatically add the necessary tallies. In this example, we will tally on
the same mesh given in the `[Mesh]` block.
Finally, we specify the level in the geometry on which the cells
exist. Because we don't have any lattices or filled universes in our OpenMC model,
the cell level is zero.

It is common when performing multiphysics with Monte Carlo solvers to use
relaxation schemes. Here, we use the Dufek-Gudowski scheme to slowly ramp
up the number of particles used in each successive OpenMC solve.

On the first time step, our OpenMC model contains five cells (one large cell each
to represent the entire first wall, multiplier, breeder, vacuum vessel, and divertor).
Because OpenMC uses surface tracking, this means that multiphysics feedback would
impose a significant constraint that the temperature and density of each of these
regions would be homogeneous (constant). However, there will exist gradients in
temperature and/or density in these components as computed by the thermal-fluid
physics. In order to more finely capture these feedback effects, we add a
[MoabSkinner](https://cardinal.cels.anl.gov/source/userobjects/MoabSkinner.html)
object in order to on-the-fly regenerated the OpenMC cells according to contours
 in temperature and/or density. Here, we will re-generate the Monte Carlo model
to create new cells for every 50 K in temperature difference.

!listing /tutorials/tokamak/openmc.i
  block=UserObjects

Next, we add a series of auxiliary variables for solution visualization
(these are not requried for coupling). To help with understanding
how the OpenMC model maps to the mesh in the `[Mesh]` block, we add auxiliary
variables to visualize OpenMC's
cell temperature ([CellTemperatureAux](/auxkernels/CellTemperatureAux.md)).
Cardinal will also automatically output a variable named `cell_id`
([CellIDAux](https://cardinal.cels.anl.gov/source/auxkernels/CellIDAux.html))
and a variable named `cell_instance` (
[CellInstanceAux](https://cardinal.cels.anl.gov/source/auxkernels/CellInstanceAux.html)) to show the spatial mapping.

!listing /tutorials/tokamak/openmc.i
  start=AuxVariables
  end=Problem

Next, we specify an executioner and output settings. Even though OpenMC technically
performs a fixed source calculation (with no time dependence), we use the transient
executioner so that *if* we wanted to run OpenMC more times than the coupled
main application via subcycling, we would have a way to control that.

!listing /tutorials/tokamak/openmc.i
  start=Executioner
  end=Postprocessors

Finally, we add a postprocessor to evaluate the total heat source computed by OpenMC
and query other parts of the solution.

!listing /tutorials/tokamak/openmc.i
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

[dagmc_heat_source2] shows the heat source computed by OpenMC (units of W/cm$^3$)
and mapped to the MOOSE mesh and the solid temperature computed by MOOSE, on the last
Picard iteration. Note that these results are not necessarily intended to replicate
a realistic tokamak, due to the highly simplified neutron source and lack of fluid flow cooling.

!media dagmc_heat_source2.png
  id=dagmc_heat_source2
  caption=Heat source (W/cm$^3$) computed by OpenMC (left); MOOSE solid temperature (right)
