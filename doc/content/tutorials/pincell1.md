# Tutorial 6A: LWR Pincell

In this tutorial, you will learn how to:

- Couple OpenMC via temperature and heat source feedback to MOOSE
- Control the resolution of physics feedback received by OpenMC
- Tally an OpenMC heat source on an unstructured mesh

!alert! note
This tutorial makes use of the following major Cardinal classes:

- [OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md)

We recommend quickly reading this documentation before proceeding
with this tutorial.
!alert-end!

This tutorial describes how to use Cardinal to perform temperature and heat
source coupling of OpenMC to MOOSE for a [!ac](LWR) pincell.
At a high level, Cardinal's wrapping of OpenMC consists of two major stages - first, establishing
the mapping between OpenMC's geometry and the [MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html)
with which OpenMC communicates. This stage consists of:

1. Mapping the elements in a [MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html)
  to OpenMC cells by identifying the OpenMC cell that resides at each element's centroid.
  The mapping does not place any requirements on geometry alignment.
2. Identifying which MOOSE mesh blocks are "solid" and which are "fluid." The solid blocks will then
  exchange temperature with OpenMC, while the fluid blocks will exchange both temperature and
  density with OpenMC.
3. If using cell tallies, identifying which MOOSE blocks should be tallied - tallies are then added to all OpenMC cells that
  correspond to those elements. If using mesh tallies, tallies are added with a unique bin
  in each mesh element.

The second stage of the wrapping encompasses the actual multiphysics solve:

1. Adding a [MooseVariable](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
  to represent OpenMC's heat source. In other words, if OpenMC stores the fission heating tally
  as a `std::vector<double>` (it doesn't, but assume it does for pedagogical reasons), with each entry corresponding to a tally bin,
  then a [MooseVariable](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
  is created that represents the same data, but mapped to the [MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html).
2. Writing multiphysics feedback fields in/out of OpenMC's internal cell and material representations.
  In other words, if OpenMC represents cell temperature as `std::vector<double>` (it doesn't, but assume it does for pedagogical reasons), this involves reading
  from a [MooseVariable](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
  representing temperature and writing into OpenMC's internal data structures. A similar process occurs for
  density feedback.

Cardinal developers have an intimate knowledge of how OpenMC stores its tally results,
temperatures, and densities, so this entire process is automated for you! OpenMC
can communicate with any other MOOSE application via the [MultiApp](https://mooseframework.inl.gov/syntax/MultiApps/index.html)
and [Transfer](https://mooseframework.inl.gov/syntax/Transfers/index.html)
systems in MOOSE. The same wrapping can be used for temperature, density, and heat source
feedback with *any* MOOSE application that can compute temperature and density -
such as Pronghorn, BISON, and even the NekRS wrapping in Cardinal.

## Geometry and Computational Models

This section describes the geometry for the present [!ac](LWR) pincell analysis. The relevant dimensions
are summarized in [table1]. The geometry consists of a UO$_2$ pincell within a Zircaloy
cladding; a helium gap separates the fuel from the cladding. Borated water
is present outside the cladding.

!table id=table1 caption=Geometric specifications for an [!ac](LWR) pincell
| Parameter | Value (cm) |
| :- | :- |
| Pellet outer radius | 0.39218 |
| Clad inner radius | 0.40005 |
| Clad outer radius | 0.45720 |
| Pin pitch | 1.25984 |
| Height | 300 |

Heat is produced in the pellet region and is transferred by radiation and conduction across
the pellet-clad gap to the cladding. A total core power of 3000 MWth is assumed uniformly
distributed among 273 fuel bundles, each with 17$\times$17 pins (neglecting the effect
of guide tubes on the average pin power). The tallies from OpenMC
will therefore be normalized according to a pin-wise power of

\begin{equation}
\label{eq:1}
q_{pin}=\frac{3000\text{\ MWth}}{n_bn_p}
\end{equation}

where $n_b=273$ is the number of fuel bundles and $n_p=289$ is the number of pins
per bundle.

### MOOSE Heat Conduction Model

!include steady_hc.md

MOOSE [MeshGenerators](https://mooseframework.inl.gov/syntax/Mesh/index.html) are used to construct
the solid mesh. [solid_mesh] shows the solid mesh with block IDs and sidesets.
Different block IDs are used for the hexahedral and prism elements
in the pellet region because libMesh does not allow different element types
to exist on the same block ID.
Because this mesh is generated using
the [MeshGenerator](https://mooseframework.inl.gov/syntax/Mesh/)
system in MOOSE, the mesh is created at runtime. If you want to generate a mesh file,
you can do so by running the solid input file in mesh generation mode:

!listing
$ cardinal-opt -i mesh.i --mesh-only

!media pincell_solid_mesh.png
  id=solid_mesh
  caption=Mesh for the solid portions of an [!ac](LWR) pincell; boundary IDs are shown to the right

Because heat transfer and fluid flow in the borated water is not modeled in this example,
heat removal by the fluid is approximated by setting
the outer surface of the cladding to a convection boundary
condition,

\begin{equation}
\label{eq:2}
q^{''}=h\left(T-T_\infty\right)
\end{equation}

where $h=1000$ W/m$^2$/K and $T_\infty$ is a function linearly increasing
from $300$&deg;C at the bottom of the pincell to $350$&deg;C at the top
of the pincell. The top and bottom of the solid pincell
are assumed insulated.

!include radiation_gap.md

In this example, the MOOSE heat conduction module will run first. The initial
solid temperature is 280&deg;C and the initial power is zero.

### OpenMC Model

The OpenMC model is built using [!ac](CSG), which are cells created from regions
of space formed by half-spaces of various common surfaces.
When creating the OpenMC
geometry, there are three aspects that you must pay attention to when using Cardinal -

1. The resolution of the temperature (and density, for fluid feedback) feedback to impose in OpenMC
2. The "level" of the cells with which you want to perform feedback
3. The manner in which temperature feedback is applied to the cross section data

The temperature in OpenMC is stored on the [!ac](CSG) cells. One constant temperature
can be set for each cell. Therefore, the resolution of the temperature feedback
received from MOOSE is determined during the OpenMC model setup. Each
OpenMC cell will receive a unique temperature, so the number of OpenMC cells dictates
the temperature feedback resolution.

The second consideration is slightly more subtle, but allows great flexibility for
imposing multiphysics feedback for very heterogeneous geometries, such as
[!ac](TRISO) pebbles. The "level" of a [!ac](CSG) cell refers to the number of
nested universes (relative to the root universe) at which you would like to impose
feedback. If you construct your geometry without *filling* any OpenMC cells with
other universes, then all your cells are at level zero - i.e. the highest level in
the geometry. But if your model contains lattices or filled universes, the level that you want to perform
multiphysics coupling on is most likely not the highest level in the geometry. For instance,
[!ac](LWR) cores are comprised of hundreds of assemblies. The simplest approach to creating this
geometry would be to make a single fuel assembly universe, then repeat that
universe several times throughout the geometry. If you wanted to apply a single temperature
to the entire assembly (pins plus coolant), then you would set a level of 1.
If your assembly itself was formed as a lattice of single-pin unit cells, then to apply
unique temperatures to each pin, you would set a level of 2. In other words, when we
establish a mapping to MOOSE, we do not simply use the *lowest*
cell level in the geometry because it is sometimes desirable to set the temperature/density
for all cells *contained* in a particular cell.

!alert note
When setting up your OpenMC coupling, we *highly* recommend running Cardinal with
`verbose` set to `true`.
This setting will display the mapping of OpenMC cells to
MOOSE elements and should help provide a grasp on the "level" concept.

OpenMC's Python [!ac](API)
is used to create the pincell model with the script shown below. First, we define
materials for the various regions and create the geometry. We add
40 cells to receive solid temperature feedback by dividing the entire axial
height by 41 axial planes.
Note that this particular choice of axial cells has *no* relationship to the solid mesh.
That is, MOOSE elements can span more than one OpenMC cell; when mapping by centroid,
however, each MOOSE element will then be associated with one OpenMC cell (which may be
an imperfect alignment, but represents a form of discretization error in the MOOSE element refinement).
For simplicity here we select the same axial discretization in the
OpenMC model as used in the `[Mesh]`.
The OpenMC geometry as produced via plots is shown in [pincell_openmc].

!listing /tutorials/lwr_solid/make_openmc_model.py

!media pincell_openmc.png
  id=pincell_openmc
  caption=OpenMC [!ac](CSG) geometry colored by cell ID shown on the $x$-$y$ and $x$-$z$ planes
  style=width:60%;margin-left:auto;margin-right:auto

The top and bottom of the pincell are vacuum boundaries.
The four lateral faces of the pincell are reflective.
Because we are not filling any universes with other universes or lattices,
all of the cells in this problem are at the highest level of the geometry -
i.e. the root universe.

We use a
linear-linear stochastic interpolation between the two cross section data sets nearest
to the imposed temperature by setting the `method` parameter on
`settings.temperature` to `interpolation`. Finally,
when OpenMC is initialized, cross section data is only loaded for a range of temperatures to save on memory usage;
this range is specified with the [temperature range](https://docs.openmc.org/en/latest/io_formats/settings.html#temperature-range-element)
element. You should set the range to be larger than the temperature range you
expect in your coupled multiphysics problem - otherwise, you will encounter
a runtime error that data is not available at the requested temperatures.

Because OpenMC runs at the end of each timestep, the initial fluid temperature
is seto to 280&deg;C. And as there is no density feedback in this example, the
densities initially imposed
in the OpenMC model remain fixed at the values set in the OpenMC input files.

To generate the XML files needed to run OpenMC, you can run the following:

```
$ python make_openmc_model.py
```

or simply use the XML files checked in to the `tutorials/lwr_solid` directory.

## Multiphysics Coupling
  id=coupling

In this section, OpenMC and MOOSE are coupled for heat source and temperature feedback
for the solid regions of an [!ac](LWR) pincell. All input files are present in the
`tutorials/lwr_solid` directory. The following sub-sections describe these files.

### Solid Input Files

The solid phase is solved with the MOOSE heat conduction module, and is
described in the `solid.i` input. The solid mesh is created using mesh generators
in the `mesh.i` input:

!listing /tutorials/lwr_solid/mesh.i

We generate the mesh by running `cardinal-opt -i mesh.i --mesh-only` to create the
`mesh_in.e` file, which we then use in the solid input file.

!listing /tutorials/lwr_solid/solid.i
  end=Variables

!alert note
OpenMC *always* has geometries set up in length units of centimeters. However,
MOOSE is dimension-agnostic, and the same physics model can be set up in any
length unit provided proper scalings are applied to material properties, source terms,
and the mesh. In this example, we set up the solid input file in length units of centimeters.

The heat conduction module will solve for temperature, which we define
as a nonlinear variable and apply a simple uniform initial condition.

!listing /tutorials/lwr_solid/solid.i
  block=Variables

The [Transfer](https://mooseframework.inl.gov/syntax/Transfers/index.html)
system in MOOSE is used to communicate variables across applications; a
heat source will be computed by OpenMC and applied as a source term in MOOSE.
In the opposite direction, MOOSE will compute a temperature that will be
applied to the OpenMC geometry. Because we already solve for temperature, we
simply need to add an auxiliary variable to receive the heat source from OpenMC.

!listing /tutorials/lwr_solid/solid.i
  block=AuxVariables

The governing equation solved by MOOSE is specified in the `[Kernels]` block with the
[HeatConduction](https://mooseframework.inl.gov/source/kernels/HeatConduction.html)
and [CoupledForce](https://mooseframework.inl.gov/source/kernels/CoupledForce.html) kernels.
The heat source provided by OpenMC is given by the receiver `heat_source` auxiliary variable.

!listing /tutorials/lwr_solid/solid.i
  block=Kernels

We also set thermal conductivity values in the pellet and clad. Constant
values are used for simplicity.

!listing /tutorials/lwr_solid/solid.i
  block=Materials

Next, the boundary conditions on the solid are applied, including the thermal
contact model between the pellet and the clad.

!listing /tutorials/lwr_solid/solid.i
  start=Functions
  end=Materials

In this example, the overall calculation workflow is as follows:

1. Run MOOSE heat conduction with a given power distribution from OpenMC.
2. Send temperature to OpenMC by modifying the temperature of OpenMC cells.
  A volume average is performed over all the MOOSE elements that mapped to
  a particular OpenMC cell.
3. Run OpenMC with an updated temperature distribution.
4. Extract the kappa-fission distribution (the recoverable fission energy)
  computed by OpenMC and map in the opposite direction from OpenMC cells
  to all the MOOSE elements that mapped to each cell.

The above sequence is repeated until the desired convergence of the coupled domain.
The [MultiApps](https://mooseframework.inl.gov/syntax/MultiApps/index.html)
and [Transfers](https://mooseframework.inl.gov/syntax/Transfers/index.html)
blocks describe the interaction between Cardinal and MOOSE. The MOOSE
heat conduction application is here run as the master application, with
OpenMC run as the sub-application. We specify that MOOSE will run
first on each time step.

Two transfers are required to couple OpenMC and MOOSE for heat source and
temperature feedback; the first is a transfer of heat source from Cardinal
to MOOSE. The second is transfer of temperature from MOOSE to Cardinal.

!listing /tutorials/lwr_solid/solid.i
  start=MultiApps
  end=Postprocessors

For the heat source transfer from OpenMC, we ensure conservation by
requiring that the integral of heat source computed by OpenMC
(in the `heat_source` postprocessor) matches the integral of the heat
source received by MOOSE (in the `source_integral` postprocessor).
We also add a postprocessor to evaluate the maximum solid temperature.

!listing /tutorials/lwr_solid/solid.i
  block=Postprocessors

Because we did not specify sub-cycling
in the `[MultiApps]` block, this means that OpenMC will run for exactly the
same number of time steps (but the actual time step size used by the OpenMC
wrapping is of no consequence because OpenMC is run in $k$-eigenvalue mode).
By setting a fixed number of time steps, this example
will simply run a fixed number of Picard iterations.

!listing /tutorials/lwr_solid/solid.i
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

!listing /tutorials/lwr_solid/openmc.i
  end=AuxVariables

Next, the [Problem](https://mooseframework.inl.gov/syntax/Problem/index.html)
block describes all objects necessary for the actual physics solve. To replace
MOOSE finite element calculations with OpenMC particle transport calculations,
the [OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md) class
is used.

!listing /tutorials/lwr_solid/openmc.i
  block=Problem

For this example, we specify the total fission power by which to normalize OpenMC's
tally results (because OpenMC's tally results are in units of eV/source particle).
Next, we indicate which blocks in the `[Mesh]` should be considered
as "solid" (and therefore send temperatures into OpenMC) with `solid_blocks`.
Here, we specify temperature feedback for the pellet (blocks 2 and 3) and the cladding
(block 1). During the initialization, [OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md)
will automatically map from MOOSE elements to OpenMC cells, and store which MOOSE elements
are "solid." Then when temperature is sent into OpenMC, that mapping is used to compute
a volume-average temperature to apply to each OpenMC cell.

!alert note
While we use nomenclature
like `solid_blocks`, these blocks don't need to actually represent
*solid* - we only use this verbage to note where OpenMC should receive temperature
feedback, but *not* density feedback (which would require moving the cell boundaries
for deforming solids to preserve mass).

This example uses cell tallies, as indicated by
`tally_type`.
The `tally_blocks` are
then used to indicate which OpenMC cells to add tallies to
(as inferred from the mapping of MOOSE elements to OpenMC cells). If not specified,
we add tallies to all OpenMC cells. But for this problem, we already know that the
cladding doesn't have any fissile material, so we can save some effort with the
tallies by skipping tallies in those regions by setting
`tally_blocks` to blocks 2 and 3.
[OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md) will then
automatically add the necessary tallies.
Finally, we specify the level in the geometry on which the solid cells
exist. Because we don't have any lattices or filled universes in our OpenMC model,
the solid cell level is zero.

Next, we add a series of auxiliary variables for solution visualization
(these are not requried for coupling). To help with understanding
how the OpenMC model maps to the mesh in the `[Mesh]` block, we add auxiliary
variables to visualize OpenMC's cell ID ([CellIDAux](/auxkernels/CellIDAux.md)),
cell instance ([CellInstanceAux](/auxkernels/CellInstanceAux.md)),
and cell temperature ([CellTemperatureAux](/auxkernels/CellTemperatureAux.md)) on
the `[Mesh]`.

!listing /tutorials/lwr_solid/openmc.i
  start=AuxVariables
  end=Problem

Next, we specify an executioner and output settings. Even though OpenMC technically
performs a criticality calculation (with no time dependence), we use the transient
executioner so that *if* we wanted to run OpenMC more times than the coupled
master application via subcycling, we would have a way to control that.

!listing /tutorials/lwr_solid/openmc.i
  start=Executioner
  end=Postprocessors

Finally, we add a postprocessor to evaluate the total heat source computed by OpenMC.
We also include a [FissionTallyRelativeError](/postprocessors/FissionTallyRelativeError.md)
postprocessor to evaluate the maximum relative error of the cell tally and a third postprocessor
to evaluate the maximum heat source.

!listing /tutorials/lwr_solid/openmc.i
  block=Postprocessors

You will likely notice that many of the always-included MOOSE blocks are not
present in the `openmc.i` input. [OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md)
automatically adds the `heat_source`, `temp`, and `density` (if density is coupled
to OpenMC) variables in the `openmc.i` input, so these will never appear in the OpenMC
wrapper file explicitly. It is as if the following is included in the input file:

!listing
[AuxVariables]
  [heat_source]
    family = MONOMIAL
    order = CONSTANT
  []
  [temp]
    family = MONOMIAL
    order = CONSTANT
  []
  [density] # only present if fluid_blocks was provided (not this example)
    family = MONOMIAL
    order = CONSTANT
  []
[]

## Execution and Postprocessing

To run the coupled calculation, run the following:

```
$ mpiexec -np 8 cardinal-opt -i solid.i --n-threads=2
```

This will run both MOOSE and OpenMC with 8 [!ac](MPI) processes and 2 OpenMP threads.
To run the simulation faster, you can increase the parallel processes/threads, or
simply decrease the number of particles used in OpenMC.
When the simulation has completed, you will have created a number of different output files:

- `solid_out.e`, an Exodus output with the solid mesh and solution
- `solid_out_openmc0.e`, an Exodus output with the OpenMC solution and the data
  that was ultimately transferred in/out of OpenMC

First, let's examine how the mapping between OpenMC and MOOSE was established.
When we run with `verbose = true`, you will see the following mapping information displayed:

```
Mapping of OpenMC cells to MOOSE mesh elements:
-----------------------------------------------------------------------------------
|             Cell              | # Solid | # Fluid | # Uncoupled | Mapped Volume |
-----------------------------------------------------------------------------------
| id   1, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id   3, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id   5, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id   7, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id   9, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id  11, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id  13, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id  15, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id  17, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id  19, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id  21, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id  23, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id  25, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id  27, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id  29, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id  31, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id  33, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id  35, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id  37, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id  39, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id  41, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id  43, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id  45, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id  47, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id  49, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id  51, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id  53, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id  55, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id  57, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id  59, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id  61, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id  63, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id  65, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id  67, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id  69, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id  71, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id  73, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id  75, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id  77, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id  79, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id  81, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id  83, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id  85, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id  87, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id  89, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id  91, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id  93, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id  95, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id  97, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id  99, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id 101, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id 103, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id 105, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id 107, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id 109, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id 111, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id 113, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id 115, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id 117, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id 119, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id 121, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id 123, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id 125, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id 127, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id 129, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id 131, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id 133, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id 135, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id 137, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id 139, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id 141, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id 143, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id 145, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id 147, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id 149, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id 151, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id 153, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id 155, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
| id 157, instance   0 (of   1) |     200 |       0 |           0 |  3.564630e+00 |
| id 159, instance   0 (of   1) |      60 |       0 |           0 |  1.135448e+00 |
-----------------------------------------------------------------------------------
```

This shows that a total of 80 OpenMC cells mapped to the MOOSE blocks (blocks 1, 2, and 3 -
because that's what we specified for `solid_blocks`). Because the gap between the pellet
and cladding is un-meshed, the helium gap in the OpenMC model does not participate in
coupling.
The above message also shows the
volume that each OpenMC cell maps to. Because there are no distributed cells in this
problem, each cell only has a single instance.

[lwr_heat_source] shows the heat source computed by OpenMC (units of W/cm$^3$)
and mapped to the MOOSE mesh;
the block corresponding to the cladding is not shown.
To the right is shown the heat source mapped along a line down the centerline of
the rod. The heat source is slightly bottom-peaked due to the negative
Doppler feedback from the fuel.

!media lwr_heat_source.png
  id=lwr_heat_source
  caption=Heat source (W/cm$^3$) computed by OpenMC

[lwr_solid_temp] shows the temperature computed by the MOOSE heat conduction
module, while [lwr_solid_temp_mc] shows the temperature actually set in the OpenMC
cells mapped to the MOOSE elements.
Because a single cell was used to represent
the cladding and fuel (at each axial layer), only one temperature is shown for the
fuel and clad regions in the `cell_temperature` auxiliary variable. The temperatures
set in OpenMC are volume averages of the temperature computed by MOOSE, i.e. the
temperature shown in [lwr_solid_temp]. If you want to resolve the solid temperature
with more detail in the OpenMC model, simply add OpenMC cells where finer feedback
is desired.

!media lwr_solid_temp.png
  id=lwr_solid_temp
  caption=Temperature computed by MOOSE, on the top of the pincell
  style=width:45%;margin-left:auto;margin-right:auto

!media lwr_solid_temp_openmc.png
  id=lwr_solid_temp_mc
  caption=Temperature set in OpenMC cells (shown in terms of the `[Mesh]`), on the top of the pincell
  style=width:45%;margin-left:auto;margin-right:auto

## Adding Mesh Tallies

Next, we will replace the cell
tallies with unstructured mesh tallies. That is, instead of setting
`tally_blocks` and
providing the MOOSE blocks to which the corresponding OpenMC cells should have tallies added,
we will tally on an unstructured mesh. The inputs for this problem are largely the
same as in [#coupling]; the files are now `solid_um.i` and `openmc_um.i`.
For the solid, we simply need to swap out the sub-application to
point to a different input file.

!listing /tutorials/lwr_solid/solid_um.i
  block=MultiApps

Then, in `openmc_um.i`, we make small modifications to the settings for the
[OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md). We indicate that
`tally_type`
is set to `mesh` and provide the unstructured mesh that we want to tally as a file
with the `mesh_template`.

!listing /tutorials/lwr_solid/openmc_um.i
  block=Problem

By default, Cardinal will normalize the OpenMC fission energy tally according
to a global tally over the entire OpenMC problem. When using mesh tallies on
curvilinear surfaces, an unstructured mesh often cannot perfectly represent the
domain. In this problem, for instance, the faceted nature of the pincell mesh
means that a (small) amount of the fission energy is omitted in the tally because
some regions of the OpenMC [!ac](CSG) cell are *outside* any of the tally mesh
elements (but still within the cylindrical pellet).
In order to still obtain the specified
`power`,
we therefore change how the tallies are normalized. Instead of normalizing by a
problem-wide tally (which includes the regions of the pellet that are outside
the unstructured mesh but inside the pellet), we normalize instead by the sum of the mesh tally itself
by setting
`normalize_by_global_tally` to `false`. This ensures that the power we
specify will be obtained when normalizing the OpenMC tally. In the limit of
an extremely refined unstructured mesh, the error in normalizing by the global
tally decreases to zero.

The mesh file we use for tallying is simply the `mesh_in.e` mesh we generated
earlier with the mesh generators.
Note that because the mesh in the `[Mesh]` block contains elements that correspond
to the cladding, we will technically be tallying in cladding regions, even though
there isn't a heat source there. Simply delete the cladding blocks in the mesh template
if this is a concern.

!alert warning
There are several important limitations in the current implementation of mesh tallies
in Cardinal - these will be relaxed in the future, but you must be aware of them with the
current state of the repository. First, if the mesh provided by the
`mesh_template`
has $N$ elements, those elements must
*exactly* match the first $N$ elements in the `[Mesh]`. The reason for this limitation is that
the heat source tally is simply written to the corresponding mesh element in the
`[Mesh]` by element index (as opposed to doing a nearest-element search). If the mesh in the
`[Mesh]` block contains both solid and fluid elements, for instance, and you only want to
tally on an unstructured mesh in the solid, all the solid elements in the `[Mesh]` should
appear first in the total combined mesh. You can use a [CombinerGenerator](https://mooseframework.inl.gov/source/meshgenerators/CombinerGenerator.html)
to achieve this if your fluid and solid meshes are saved in separate files
or if you use separate mesh generators for the phases. We have
checks in place to make sure you don't inadvertently bypass this requirement.

As some of the mesh tally bins are quite small, first increase the number
of inactive batches from 500 to 1000 and the number of total batches from 1500
to 10000. Instead of re-running the `make_openmc_model.py` script, we can
directly control these settings in the MOOSE-wrapped input file with
the `particles`,
`inactive_batches`, and
`batches` parameters.
Then, to run the input using mesh tallies, use:

```
$ mpiexec -np 24 cardinal-opt -i solid_um.i --n-threads=2
```

[mesh_hs] shows the unstructured mesh heat source computed by OpenMC; the
clad region is shown as solid gray. You can
see the "rim effect" common in [!ac](LWR) fuels,
where the highest power
is observed near the very edge of the fuel pellet. Because the tally bins
are quite small, further increases
in the total number of particles simulated will reduce the azimuthal asymmetry
in the tally results in this azimuthally-symmetric geometry.

!media mesh_hs.png
  id=mesh_hs
  caption=Unstructured mesh heat source computed by OpenMC, shown on the midplane of the pincell, with 20000 particles per batch, 1000 inactive batches, and 10000 total batches
  style=width:45%;margin-left:auto;margin-right:auto

Note that adding unstructured mesh tallies only affects how the
heat source is measured in OpenMC - the use of unstructured mesh tallies has no
bearing on the temperature and density resolution going *into* OpenMC. For this
example, the temperature will have the same resolution as shown in
[lwr_solid_temp_mc], albeit with slightly different values because the use of
unstructured mesh tallies changes the coupled solution
due to the different resolution of the heat source.
