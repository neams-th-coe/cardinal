# Tutorial 6B: TRISO Pebbles in a Lattice

In this tutorial, you will learn how to:

- Couple OpenMC via temperature and heat source feedback to MOOSE
- Establish coupling between OpenMC and MOOSE for nested universe OpenMC models
- Couple OpenMC solves in units of centimeters with MOOSE solves in units of meters
- Repeat the same mesh tally several times throughout the OpenMC domain
- Apply temperature feedback from homogenized thermal-hydraulics models

!alert! note
This tutorial makes use of the following major Cardinal classes:

- [OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md)

We recommend quickly reading this documentation before proceeding
with this tutorial. If you have not completed
[Tutorial 6A](pincell1.md), please first read that tutorial to gain most
of the background information related to coupling OpenMC and MOOSE. This
tutorial also requires you to download a mesh file from Box. Please download
the files from the `pebbles` folder [here](https://anl.app.box.com/folder/141527707499?s=irryqrx97n5vi4jmct1e3roqgmhzic89)
and place these files within the same directory structure in `tutorials/pebbles`.
!alert-end!

This tutorial describes how to use Cardinal to perform temperature and heat
source coupling of OpenMC and MOOSE for modeling [!ac](TRISO) fuel pebbles,
each of which is a very heterogeneous mixture of millimeter-size fuel particles
in a graphite matrix.
Cardinal contains convenient features for applying multiphysics
feedback to heterogeneous domains, when a coupled physics application (such as MOOSE
heat conduction) might *not* also resolve the heterogeneities. For instance, the
fuel pebble model in the coarse-mesh thermal-hydraulic tool Pronghorn
[!cite](novak2021b) uses the Heat Source Decomposition method to predict pebble
interior temperatures, which does not explicitly resolve the [!ac](TRISO) particles
in the pebble. Before describing this homogenized temperature feedback, this tutorial
first demonstrates multiphysics coupling for solid UO$_2$ spheres. The second part
of this tutorial, in [#simplified], then addresses homogenized feedback.

## Geometry and Computational Model

The first application in this tutorial will be to coupling OpenMC and MOOSE heat
conduction for solid UO$_2$ spheres (i.e. homogeneous spheres). The geometry and
computational model for this case consists of a vertical "stack"
of three pebbles, each of 1.5 cm radius. The pebble centers
are located at $(0, 0, 2)$ cm, $(0, 0, 6)$ cm, and $(0, 0, 10)$ cm.
FLiBe coolant occupies the space around the pebbles. Heat is produced in the
pebbles; we assume the total power is 1500 W.

## Boundary Conditions

This section describes the boundary conditions imposed on the Monte Carlo particle transport model and
the MOOSE heat conduction model.

#### Neutronics Boundary Conditions

For the neutronics physics, the pebbles are placed in a box with $x$-$y$ width
of 4$\times$4 cm and height of 12 cm. All boundaries of this box are reflecting.

#### Solid Boundary Conditions

Because heat transfer and fluid flow in the FLiBe is not modeled in this example,
heat removal by the fluid is approximated by setting the outer surface of the
pebble to a convection boundary condition,

\begin{equation}
\label{eq:1}
q^{''}=h\left(T-T_\infty\right)
\end{equation}

where $h=1000$ W/m$^2$/K and $T_\infty=650$&deg;C.

## Initial Conditions

The initial temperature is 650&deg;C, while the initial heat source in the solid is zero.
Because there is no density feedback in this example, the densities initially imposed
in the OpenMC model remain fixed at the values set in the OpenMC input files.

## Meshing

This section describes the mesh used for the solid domain.
In this problem, we will solve MOOSE in units of meters, which is convenient
for heat transfer applications because
material properties for thermal-fluids physics are almost always given in SI units. Further,
many MOOSE physics modules inherently assume SI units, such as the
[fluid property module](https://mooseframework.inl.gov/modules/fluid_properties/index.html).
The solid mesh (which is in a length unit of meters) is shown in [solid_mesh].
The only sideset in the domain is the surface of the pebbles, which is sideset 1.

!media pebble_mesh.png
  id=solid_mesh
  caption=Mesh for solid domain
  style=width:15%;margin-left:auto;margin-right:auto

## CSG Geometry

This section describes the [!ac](CSG) model setup in OpenMC.
All OpenMC geometries currently use CSG to describe the geometry, where cells are created
from half-spaces of various common surfaces. Because this is a Cardinal tutorial,
we assume you have some basic familiarity with OpenMC, so we only discuss the portions
of the model setup relevant to multiphysics feedback - a detailed description of OpenMC
model setup is available on the [OpenMC documentation website](https://docs.openmc.org/en/stable/).

In this tutorial, we will build OpenMC's geometry using *lattices*.
A lattice is a structured repetition
of universes that is often used to simplify model setup.
In some cases, lattices are used to accelerate geometric searches as particles travel through the model.
Here, we create a repeatable universe consisting of a single pebble and surrounding FLiBe,
and then we repeat it three times throughout the geometry.

OpenMC's Python [!ac](API) is used to create the pebbles model with the script shown below.
First, we define materials for the various regions and create the geometry.
We create a universe consisting of a single pebble, and then repeat that universe
three times in a lattice. Because we have one cell per pebble, each pebble will
receive a single temperature from a coupled MOOSE application.
Because we repeat the pebble universe three times in the geometry,
the solid cell level is 1. If we were to specify level zero in the geometry, we would apply feedback
to the `main_cell`, which would apply temperature feedback across the problem globally. We instead want to apply feedback to each pebble individually. The OpenMC geometry as produced via plots is shown in [mc_geom].

!listing /tutorials/pebbles/make_openmc_model.py

!media pebble_openmc.png
  id=mc_geom
  caption=OpenMC [!ac](CSG) geometry (colored by cell ID)
  style=width:15%;margin-left:auto;margin-right:auto

As you can see, there
are only two unique cell *IDs*. This problem is built using *distributed cells*, meaning a cell with the same *ID* appears multiple times in the geometry; each time the cell is repeated, we assign
a new *instance* to that cell. So, the three pebbles are represented as:

- Cell ID 1, instance 0
- Cell ID 1, instance 1
- Cell ID 1, instance 2

and the FLiBe region is represented as:

- Cell ID 2, instance 0
- Cell ID 2, instance 1
- Cell ID 2, instance 2

## Multiphysics Coupling

In this section, OpenMC and MOOSE are coupled for heat source and temperature feedback for
the solid regions of a stack of three pebbles. All input files are present in the
`tutorials/pebbles` directory. The following sub-sections describe these files.

### Solid Input Files

The solid phase is solved with the MOOSE heat conduction module, and is described in the
`solid.i` input. We set up the mesh using the [CombinerGenerator](https://mooseframework.inl.gov/source/meshgenerators/CombinerGenerator.html) to translate a single sphere mesh to multiple locations.

!listing /tutorials/pebbles/solid.i
  block=Mesh

Next, we define the temperature variable, `temp`, and specify the governing equations
and boundary conditions that we will apply. We set the thermal conductivity of the pebbles
to 50 W/m/K.

!listing /tutorials/pebbles/solid.i
  start=Variables
  end=AuxVariables

The heat source received from OpenMC is stored in the `heat_source` auxiliary variable.

!listing /tutorials/pebbles/solid.i
  block=AuxVariables

Finally, we specify that we will run a OpenMC as a sub-application,
with data transfers of heat source from OpenMC and temperature to OpenMC and use a transient
executioner.

!listing /tutorials/pebbles/solid.i
  start=MultiApps

### Neutronics Input Files

The neutronics physics is solved over the entire domain using OpenMC. The OpenMC wrapping
is described in the `openmc.i` input file. We begin by defining a mesh mirror
on which OpenMC will receive temperature from the coupled
MOOSE application, and on which OpenMC will write the fission heat source.

!listing /tutorials/pebbles/openmc.i
  block=Mesh

!alert note
Even though OpenMC solves in units of centimeters, the mesh put in the `[Mesh]` block
*must* use the same units as in the coupled MOOSE application. Otherwise, the transfers
used to send temperature and heat source to/from OpenMC will not map to the correct
elements across applications. For instance, if the `[Mesh]` in the `solid.i` input were
in units of meters, but the `[Mesh]` in the `openmc.i` input were in units of centimeters,
then a point $(0, 0, 0.02)$ m in `solid.i` would get mapped to the node closest to the point
$(0, 0, 0.02)$ cm in `openmc.i`.

Next, we define auxiliary variables that will help us visualize the cell IDs
([CellIDAux](/auxkernels/CellIDAux.md)),
cell instances ([CellInstanceAux](/auxkernels/CellInstanceAux.md)),
and cell temperatures ([CellTemperatureAux](/auxkernels/CellTemperatureAux.md))in OpenMC.

!listing /tutorials/pebbles/openmc.i
  start=AuxVariables
  end=Problem

The [Problem](https://mooseframework.inl.gov/syntax/Problem/)
block is then used to specify the OpenMC wrapping. We define a total power of
1500 W, and indicate that we'd like to add tallies on block 0, which corresponds to the pebbles.
Because we want to tally separate fission powers for each pebble, we cannot have a cell-type
[tally filter](https://docs.openmc.org/en/stable/usersguide/tallies.html#filters). If we *did*
use a cell-type filter, then we would tally all three pebbles as the same region in space,
because all three pebbles have ID 1. Instead, we specify a `cell_instance` filter, which will
add tallies for each unique cell ID$+$instance combination. Finally, because the repeated pebble cells we'd like to tally in are repeated in the lattice nested one level below the root universe, we set the `solid_cell_level = 1`.

!listing /tutorials/pebbles/openmc.i
  block=Problem

The `scaling` parameter is used to indicate a multiplicative factor that should be
applied to the `[Mesh]` in order to get to units of centimeters. This scaling factor is
applied within the `OpenMCCellAverageProblem::findCell` routine that maps MOOSE elements to OpenMC [!ac](CSG) cells -
no actual changes are made to the mesh in the `[Mesh]` block.
Because the `[Mesh]` is in units of meters, we set `scaling = 100.0`.
The scaling is also applied to ensure that the heat source is on the correct
per-unit-volume scale that is expected by the `[Mesh]`.

Finally, we set a transient executioner, specify an Exodus output, and
define several postprocessors.

!listing /tutorials/pebbles/openmc.i
  start=Executioner

In addition to the wrapping of OpenMC in the `openmc.i` input file, we need to create the
XML input files used to run OpenMC from the Python script shown earlier. You can
run the script to generate the input files with:

```
$ python make_openmc_model.py
```

## Execution and Postprocessing

To run the coupled calculation, run the following from the command line.

```
$ mpiexec -np 8 cardinal-opt -i solid.i --n-threads=2
```

This will run both MOOSE and OpenMC with 8 [!ac](MPI) processes and 2 OpenMP threads.
When the simulation has completed, you will have created a number of different output files:

- `solid_out.e`, an Exodus II output file with the solid mesh and solution
- `solid_out_openmc0.e`, an Exodus II output file with the OpenMC solution and the data
  that was ultimately transferred in/out of OpenMC

First, let's examine how the mapping between OpenMC and MOOSE was established.
When we run with `verbose = true`, you will see the following mapping information displayed:

```
cell 1, instance 0 (of 3): 256 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  13.2213
cell 1, instance 1 (of 3): 256 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  13.2213
cell 1, instance 2 (of 3): 256 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  13.2213
```

The three cells representing the pebbles are mapped to the corresponding MOOSE elements for each pebble.
Shown below is the heat source computed by OpenMC (units of W/m$^3$) and mapped to the MOOSE mesh,
along with the temperature computed by MOOSE (before and after being averaged and sent to OpenMC cells).
The temperature is shown on a cut plane through the centers of the pebbles;
because one temperature is set for each pebble, the temperature in OpenMC for
each pebble is an average over the pebble.
Due to the reflecting nature of the problem and the identical pebble material
properties, the power of each pebble is nearly identical (the tally uncertainty
contributes to small differences).
Note that the range for the cell temperature only spans 0.02 K - that is,
the small tally uncertainty results in un-equal pebble powers that also
results in unequal pebble average temperatures. Small errors in the numerical
finite element heat conduction solution (such as only converging the solid
solution to a residual of $10^{-10}$) also result in an imperfect
correspondence between pebble power and pebble average temperature.

!media pbr_solution.png
  id=pbr_solution
  caption=Heat source and temperature computed by a coupled OpenMC-MOOSE model of pebbles

Finally, you can confirm that the `scaling` factor was applied correctly by comparing the
`heat_source` postprocessor, of the volume integral of the heat source, with the total power
specified in [OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md) - both
are equal to 1500 W. If the
integral is off by a factor of $100^3$ from the `power`, then you know right away that the
`scaling` was not applied correctly.

## Repeated Mesh Tallies
  id=um

Many geometries of interest to nuclear reactor analysis contain repeated structures -
such as pebbles in a [!ac](PBR) or pincells in a [!ac](LWR). In this section, we build
upon our model by using an unstructured mesh tally, with a single unstructured mesh
translated multiple times throughout the OpenMC geometry. In other words, if the memory
used to store an unstructured mesh of a single pebble is 500 kB, then an unstructured mesh
tally of 300,000 pebbles still only requires a mesh storage of 500 kB (as opposed to generating
a mesh of 300,000 pebbles that is ~150 GB). This stage of the tutorial also provides important explanations necessary
when combining unstructured mesh tallies with a MOOSE application that uses a length scale
other than centimeters.

The files for this stage of the coupling are the
`solid_um.i` and `openmc_um.i` inputs in the `tutorials/pebbles` directory.
For the solid, we simply need to swap out the sub-application to point to a different OpenMC
input file. We also use a finer solid pebble mesh.

!listing /tutorials/pebbles/solid_um.i
  block=MultiApps

Let's begin with the OpenMC wrapping, in `openmc_coarse.i`. The only changes required are
that we set a mesh tally type, provide a mesh template with the mesh, and specify the
translations to apply to replicate the mesh at the desired end positions in OpenMC's domain.

!listing /tutorials/pebbles/openmc_um.i
  block=Problem

!alert warning
The mesh template and mesh translations must always be in units of centimeters,
regardless of the units of the mesh in the `[Mesh]` block, because these two
parameters are used directly in OpenMC (which is always in units of centimeters).

Because our sphere mesh does not perfectly preserve the volume of the sphere
[!ac](CSG) cells, we also set `normalize_by_global_tally` to false so that we normalize only
by the sum of the mesh tally. Otherwise, we would miss a small amount of power produced
within the [!ac](CSG) spheres, but slightly outside the faceted surface of the sphere mesh. Setting this parameter to false ensures that the tally normalization is correct in that the heat sources are normalized by a tally sum over the same tally domain in the OpenMC model.

To run this input, enter the following in a command line.

```
$ mpiexec -np 8 cardinal-opt -i solid_um.i --n-threads=2
```

[mesh3] shows the temperature computed by MOOSE, the heat source *received* by MOOSE,
the temperature *applied* to OpenMC, and the heat source computed by OpenMC. The MOOSE
transfers handle the different meshes in use seamlessly. The mesh translation feature allows
a single pebble mesh to be translated to three individual positions in the OpenMC tally.
Note that the heat sources on the OpenMC `[Mesh]` and on the MOOSE heat conduction mesh
are shown on a different color scale - the volumetric power density on the MOOSE mesh is
lower than that on the OpenMC mesh elements have smaller volumes than the elements mapped
to on the MOOSE mesh (and the conservative [MultiAppNearestNodeTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppNearestNodeTransfer.html)
conserves total integrated power).
Due to small statistical errors in the tallies, the pebble averaged temperatures
differ by about 0.1 K.
Finally, recall that adding unstructured mesh tallies does not affect the resolution of temperature and
density feedback sent to OpenMC - this resolution is controlled by the cell definitions
when constructing the OpenMC input.

!media pebble_hs_mesh3.png
  id=mesh3
  caption=Temperature and heat source distributions shown on the different MOOSE and OpenMC meshes. The temperatures are shown on a slice through the centers of the pebbles.

!bibtex bibliography
