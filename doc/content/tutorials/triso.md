# Tutorial 2B: TRISO Pebbles in a Lattice

In this tutorial, you will learn how to:

- Couple OpenMC via temperature and heat source feedback to MOOSE
- Establish coupling between OpenMC and MOOSE for nested universe OpenMC models
- Couple OpenMC solves in units of centimeters with MOOSE solves in units of meters
- Repeat the same mesh tally several times throughout the OpenMC domain
- Use a different heat source mesh than a coupled MOOSE application
- Apply temperature feedback from homogenized thermal-hydraulics models

This tutorial describes how to use Cardinal to perform temperature and heat
source coupling of OpenMC and MOOSE for modeling fuel pebbles in a lattice.

!alert note
This tutorial assumes that you have read [Tutorial 2A](pincell1.md) to give most of the
background for how the OpenMC-MOOSE temperature and heat source coupling works.

## Geometry and Computational Model

In this tutorial, we will consider multiphysics feedback for [!ac](TRISO) pebbles,
which are very heterogeneous mixtures of millimeter-size fuel particles in a
graphite matrix. Cardinal contains convenient features for applying multiphysics
feedback to heterogeneous domains, when a coupled physics application (such as MOOSE
heat conduction) might *not* also resolve the heterogeneities. For instance, the
fuel pebble model in the coarse-mesh thermal-hydraulic tool Pronghorn
[!cite](novak2021b) uses the Heat Source Decomposition method to predict pebble
interior temperatures, which does not explicitly resolve the [!ac](TRISO) particles
in the pebble. Of course your thermal-hydraulics application might explicitly resolve
[!ac](TRISO) particles, and this Cardinal feature might not be of interest. But, if
you want to incorporate multiphysics feedback from a homogenized thermal-hydraulics
application, please be sure to read [#simplified].

For now, we first need to introduce other modeling features. Therefore, our beginning
application will not be to [!ac](TRISO) pebbles, but rather to solid UO$_2$ spheres.
The [!ac](TRISO) pebble case will be addressed in [#simplified] to help illustrate
features for homogenized temperature feedback.'

We will model a "stack" of three pebbles, each of 1.5 cm radius. The pebble centers
are located at $(0, 0, 2)$, $(0, 0, 6)$, and $(0, 0, 10)$, i.e. they are vertically
stacked. FLiBe coolant occupies the space around the pebbles. Heat is produced in the
pebbles; we assume the total power is 1500 W. Different enrichments will be specified
for each pebble.

## Boundary Conditions

For the neutronics physics, the pebbles are placed in a box with $x$-$y$ width
of 4$\times$4 cm and height of 12 cm. All boundaries of this box are reflecting.

For the solid physics, 
because heat transfer and fluid flow in the FLiBe is not modeled in this example,
we approximate the effect of heat removal by the fluid by setting the outer
surface of the pebble to a convection boundary condition,

\begin{equation}
\label{eq:1}
q^{''}=h\left(T-T_\infty\right)
\end{equation}

where $h=1000$ W/m$^2$/K and $T_\infty=650$&deg;C.

## Initial Conditions

The initial temperature is 650&deg;C, while the initial heat source in the solid is zero.

## Meshing

This section describes the mesh used for the solid domain. Cubit [!cite](cubit) meshing
software is used. In this problem, we will solve MOOSE in units of meters (OpenMC must always
solve in units of centimeters). This is often convenient for heat transfer applications because
material properties for thermal-fluids physics are almost always given in SI units. Further,
many MOOSE physics inherently assume SI units, such as the
[fluid property module](https://mooseframework.inl.gov/modules/fluid_properties/index.html)
and Pronghorn's solid property correlations [!cite](pronghorn_manual). To solve MOOSE in units
of meters, we will need to create a mesh in units of meters. This mesh is shown
in [solid_mesh]. The only sideset in the domain is the surface of the pebbles, which is named `1`.

!media pebble_mesh.png
  id=solid_mesh
  caption=Mesh for a stack of three pebbles
  style=width:15%;margin-left:auto;margin-right:auto

## CSG Geometry

This section describes the [!ac](CSG) model setup in OpenMC. In the near future, OpenMC will
have unstructured mesh tracking capabilities - but until that is available, OpenMC
geometries all use the [!ac](CSG) geometry approach, where cells are created from half-spaces
of various common surfaces. This tutorial differs from [Tutorial 2A](pincell1.md)
in that we will build OpenMC's geometry using *lattices*. A lattice is an ordered repetition
of universes that is often used to simplify model setup. Please consult
the OpenMC [lattice documentation](https://docs.openmc.org/en/stable/usersguide/geometry.html#lattices)
for more details. You might also have lattices in order to accelerate the geometric
search.

Here, we create a repeatable universe consisting of a single pebble and surrounding flibe,
and then we repeat it three times throughout the geometry. The Python script used to generate
the OpenMC problem is shown below.

!listing /tutorials/pebbles/make_openmc_model.py

As introduced in [Tutorial 2A](pincell1.md), there are two important aspects of the
OpenMC geometry creation in this multiphysics context -

- The resolution of the temperature (and density, for fluid feedback) to impose in OpenMC
- The "level" of the cells with which you want to perform feedback

Here, because we define a single cell for each pebble, we are going to apply a single
temperature to each pebble cell (you can of course use as many cells as you would like).
And because we're using a lattice where we repeat the `repeatable_univ` three times in the
geometry, the level of the cells is *not* level zero. On level zero of the geometry,
we are in the `main_cell`, which is filled with a lattice. On level 1 of the geometry,
we are in the `repeatable_univ`, or the universe representing our repeated geometry unit -
a single pebble plus surrounding flibe. The OpenMC geometry as produced via plots is shown below.

!media pebble_openmc.png
  id=mc_geom
  caption=OpenMC geometry (colored by cell ID) for a stack of three pebbles
  style=width:15%;margin-left:auto;margin-right:auto

In [Tutorial 2A](pincell1.md), you may recall that we had unique cell IDs for
all regions of the problem - because we divided the geometry into 12 axial layers, we
had 12 unique fuel cells, 12 unique cladding cells, 12 unique gap cells, and 12 unique fluid
cells. Here, [mc_geom] is also colored by cell, but instead we see only two unique cell
*IDs*. This problem is built using *distributed cells*, meaning that we repeat the same
cell *ID* multiple times throughout the geometry; each time the cell is repeated, we assign
a new *instance* to that cell. So, the three pebbles are represented as:

- Cell ID 1, instance 0
- Cell ID 1, instance 1
- Cell ID 1, instance 2

and the flibe region is represented as:

- Cell ID 2, instance 0
- Cell ID 2, instance 1
- Cell ID 2, instance 2

Because our repeated universe is nested one layer below the root universe, we need
to set `solid_cell_level = 1` when we set up the OpenMC wrapping.

## Multiphysics Coupling

In this section, OpenMC and MOOSE are coupled for heat source and temperature feedback for
the solid regions of a stack of three pebbles. All input files are present in the
`tutorials/pebbles` directory. The following sub-sections describe these files.

### Solid Input Files

The solid phase is solved with the MOOSE heat conduction module, and is described in the
`solid.i` input. We set up the mesh using the [CombinerGenerator](https://mooseframework.inl.gov/source/meshgenerators/CombinerGenerator.html).

!listing /tutorials/pebbles/solid.i
  block=Mesh

Next, we define the temperature variable, `temp`, and specify the governing equations
and boundary conditions that we will apply. We set the thermal conductivity of the pebbles
to 50 W/m/K - note that this heat conduction input is in SI units!

!listing /tutorials/pebbles/solid.i
  start=Variables
  end=AuxVariables

The heat source received from OpenMC is stored in the `heat_source` auxiliary variable.

!listing /tutorials/pebbles/solid.i
  block=AuxVariables

Finally, we specify that we will run an [OpenMCApp](/base/OpenMCApp.md) as a sub-application,
with data transfers of heat source from OpenMC and temperature to OpenMC and use a transient
executioner.

!listing /tutorials/pebbles/solid.i
  start=MultiApps

### Neutronics Input Files

The neutronics physics is solved over the entire domain using OpenMC. The OpenMC wrapping
is described in the `openmc.i` input file. Although OpenMC does not track particles on a mesh,
we begin by defining a mesh on which OpenMC will receive temperature from the coupled
MOOSE application, and on which OpenMC will write the fission heat source. For the time being,
we use exactly the same mesh as the coupled MOOSE application; we will relax this assumption
in [#different_meshes].

!listing /tutorials/pebbles/openmc.i
  block=Mesh

!alert note
Even though OpenMC solves in units of centimeters, the mesh put in the `[Mesh]` block
*must* use the same units as in the coupled MOOSE application. Otherwise, the transfers
used to send temperature and heat source to/from OpenMC will not map to the correct
elements across applications. For instance, if the `[Mesh]` in the `solid.i` input were
in units of meters, but the `[Mesh]` in the `openmc.i` input were in units of centimeters,
then a point $(0, 0, 0.02)$ m in `solid.i` would get mapped to the point
$(0, 0, 0.02)$ cm in `openmc.i` when using the [MultiAppCopyTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppCopyTransfer.html)
and [MultiAppMeshFunctionTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppMeshFunctionTransfer.html)
in the `solid.i` input file.

Next, we define auxiliary variables that will help us visualize the cell IDs,
instances, and temperatures in OpenMC.

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
add tallies for each unique cell ID$+$instance combination. Finally, because we have repeated
lattices nested one level below the root universe, we set the `solid_cell_level = 1`.

!listing /tutorials/pebbles/openmc.i
  block=Problem

The `scaling` parameter is used to indicate what multiplicative factor should be
applied to the `[Mesh]` in order to get to units of centimeters. This scaling factor is
applied within a `findCell` routine that maps MOOSE elements to OpenMC [!ac](CSG) cells,
and is all that is needed in order to run MOOSE in units of meters and OpenMC in units
of centimeters. Because the `[Mesh]` is in units of meters, we set `scaling = 100.0`.
The scaling is also applied to ensure that the heat source is on the correct
per-unit-volume scale that is expected by the `[Mesh]`.

Finally, the executioner, output, and postprocessors are the same as those used in
[Tutorial 2A](pincell1.md).

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

The three cells representing the pebbles are correctly mapped to the corresponding MOOSE elements.
Shown below is the heat source computed by OpenMC (units of W/m$^3$) and mapped to the MOOSE mesh,
along with the temperature computed by MOOSE (before and after being averaged and sent to OpenMC cells).

!media pbr_solution.png
  id=pbr_solution
  caption=Heat source and temperature computed by a coupled OpenMC-MOOSE model of pebbles

Due to the reflecting nature of the problem, as you increase the number of particles,
you will see the pebble powers converge on identical values - the only reason that you
can observe different pebble powers is because the heat source has fairly high uncertainty.

Finally, you can confirm that the `scaling` factor was applied correctly by comparing the
`heat_source` postprocessor, of the volume integral of the heat source, with the total power
specified in [OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md). If the
integral is off by a factor of $100^3$ from the `power`, then you know right away that the
`scaling` was not applied correctly.

## Repeated Mesh Tallies
  id=um

Many geometries of interest to nuclear reactor analysis contain repeated structures -
such as pebbles in a [!ac](PBR) or pincells in a [!ac](LWR). In [Tutorial 2A](pincell1.md),
we described how to tally OpenMC's fission heat source on an unstructured mesh. To do so,
we needed to provide a `mesh_template`, or the file containing the mesh that we want to
tally on. Cardinal has features that enable you to repeat the same unstructured mesh
multiple times throughout the OpenMC geometry - so that if we only have a mesh of a single
pebble, we can avoid the huge memory overhead of forming a mesh of 300000 pebbles and instead
only store a single mesh in memory (the mesh for a single pebble). This stage of the tutorial
describes how to repeat the same mesh multiple times throughout the OpenMC geometry for
multiphysics coupling. This stage of the tutorial also provides important explanations necessary
when combining unstructured mesh tallies with a MOOSE application that uses a length scale
other than centimeters.

The files for this stage of the coupling are the
`solid_um.i` and `openmc_um.i` inputs.
For the solid, we simply need to swap out the sub-application to point to a different OpenMC
input file.

!listing /tutorials/pebbles/solid_um.i
  block=MultiApps


Let's begin with the OpenMC wrapping, in `openmc_coarse.i`. The only changes required are
that we set a mesh tally type, provide a mesh template with the mesh, and specify a number of
translations to apply to the mesh to move the mesh to the desired end positions in OpenMC's domain.

!listing /tutorials/pebbles/openmc_um.i
  block=Problem

!alert warning
When the `[Mesh]` block is not in units of centimeters, you still *must* provide a mesh
template in units of centimeters. This is why we specify a mesh file, `sphere_in_cm.e` for
the template instead of `sphere_in_m.e`. The `mesh_translations` also must be specified in
units of centimeters, because these translations are passed directly to OpenMC (which always
solves in units of centimeters).

You can alternatively provide a `mesh_translations_file` that contains all the translations
in a text format.

Because our sphere mesh does not perfectly preserve the volume of the sphere
[!ac](CSG) cells, we also set `normalize_by_global_tally` to false so that we normalize only
by the sum of the mesh tally (otherwise, we would miss a small amount of power produced
within the [!ac](CSG) spheres, but outside the faceted surface of the sphere mesh).

To run this input, enter the following in a command line.

```
$ mpiexec -np 8 cardinal-opt -i solid_um.i --n-threads=2
```

Below is shown the heat source computed using an unstructured mesh tally by OpenMC; due to
the small number of particles, the uncertainty in these tallies are very high, so differences
from element-to-element are quite high. You can confirm that the combination of an unstructured
tally mesh in units of centimeters, but a `[Mesh]` in units of meters, functions correctly by
comparing the `heat_source` postprocessor (which is integrated over the `[Mesh]`, *not* the
`mesh_template`) to the specified `power`.

!media pebble_hs_mesh.png
  id=pebble_hs_mesh
  caption=OpenMC recoverable fission heat source tallied on an unstructured mesh
  style=width:30%;margin-left:auto;margin-right:auto

Recall that adding unstructured mesh tallies does not affect the resolution of temperature and
density feedback sent to OpenMC - this resolution is controlled by the cell definitions
when constructing the OpenMC input.

## Different Meshes
  id=different_meshes

Up until this point, exactly the same mesh has been used in the MOOSE heat conduction
module (i.e. in the `solid.i` input file) and in the OpenMC wrapping (i.e. in the
`openmc.i` input file). Using exactly the same mesh, and a `CONSTANT MONOMIAL` receiver
variable for heat source in the coupled MOOSE application let us use the
`MultiAppCopyTransfer` to get the heat source from OpenMC into the coupled MOOSE application
(this particular transfer literally copies the solution from one application to another,
but can only be used when the mesh and variable basis are exactly the same).

When we use unstructured mesh tallies in OpenMC, as was introduced in
[Tutorial 2A](pincell1.md), we require that the unstructured mesh tally be exactly the same
as the `[Mesh]` used in the OpenMC wrapping. As we saw in [Tutorial 2A](pincell1.md),
if the elements in the unstructured mesh tally are small, then we require lots of particles to
get acceptable tally statistics. But we may not want to use a very coarse mesh for the coupled
thermal solver, either. So, this part of the example modifies this tutorial by using a coarse mesh
for tallying in OpenMC, that is coupled to a finer mesh where the thermal solution occurs.
We build upon the unstructured mesh example in [#um].

For this case, our OpenMC wrapping remains unchanged from that shown in [#um] - we
specify mesh tallies by repeating a single mesh three times throughout the geometry (once for
each pebble). Instead, we now apply a uniform refinement to the mesh used for computation in
the solid model. The inputs for this example are `solid_fine.i` and `openmc_um.i`.
In the solid input file, we use a sphere mesh that was generated entirely separately
from `sphere_in_m.e` and has more elements.

!listing /tutorials/pebbles/solid_fine.i
  block=Mesh

Then, the only other required change occurs in the transfer for the heat source. Because
the mesh used for the solid phase no longer exactly matches the `[Mesh]` used in the OpenMC
wrapping for storing/receiving heat sources and temperatures, we cannot use the
[MultiAppCopyTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppCopyTransfer.html),
because a copy transfer requires the meshes to be exactly the same. Instead,
we use a [MultiAppNearestNodeTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppMeshFunctionTransfer.html).
Now, we compute a heat source by OpenMC and *normalize* it according to the volume of
the elements of the `[Mesh]` in the `openmc_um.i` input file. So, when we transfer the heat
source to a coupled MOOSE application on a different mesh, it's very likely that the integral of
the heat source is not preserved (because the meshes are different). So, we simply need to
specify the postprocessors that we'd like to "preserve" during the transfer - we want the
integral of the heat source on the `[Mesh]` in `solid_fine.i` to match the integral of
the heat source on the `[Mesh]` in `openmc_um.i`. This ensures that, even though we use
different meshes in the coupled MOOSE application, we still preserve the specified power
in the `power` parameter for [OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md).
We enforce conservation by specifying the `from_postprocessors_to_be_preserved` and
`to_postprocessors_to_be_preserved` parameters.

!listing /tutorials/pebbles/solid_fine.i
  block=Transfers

To run this input, enter the following in a command line.

```
$ mpiexec -np 8 cardinal-opt -i solid_fine.i --n-threads=2
```

[mesh3] shows the temperature computed by MOOSE, the heat source *received* by MOOSE,
the temperature *applied* to OpenMC, and the heat source computed by OpenMC. The MOOSE
transfers handle the different meshes in use seamlessly. Again, due to the very small
number of particles in this tutorial, there is high uncertainty in the tally results, so
the coupled results won't look particularly realistic unless more particles are used.

!media pebble_hs_mesh3.png
  id=mesh3
  caption=Temperature and heat source distributions shown on the different MOOSE and OpenMC meshes

## Homogenized Temperature Feedback
  id=simplified

In this last section of this tutorial, we change the OpenMC model to consider
resolved [!ac](TRISO) particles in the pebbles. We will keep the MOOSE heat conduction model
the same - that is, [!ac](TRISO) particles are not resolved in the heat conduction model,
such that the temperature exchanged with the OpenMC model actually represents some type
of volume average over the materials in a pebble.

!alert note
If your application includes very heterogeneous domains such as [!ac](TRISO) fuels,
you do *not* need to model the multiphysics coupling in this manner (i.e. not explicitly
resolving the temperatures in different materials in the thermal model). If you resolve the
[!ac](TRISO) particles in your MOOSE heat conduction model, then all of the information in
the previous tutorials applies, and you simply will exchange temperatures with a
corresponding [!ac](TRISO)-resolved OpenMC model.

Our [!ac](TRISO) pebble will also have a radius of 1.5 cm, but will be entirely filled
with [!ac](TRISO) particles at 40% packing fraction (we neglect the graphite shell that
is usually present on the outside of the particle-filled region for simplicity in this tutorial).

First, we need to create a new OpenMC model. A script for doing so is shown below.
Importantly, the cell instance filter cannot be used with cells not filled with a material -
this means that, because we do indeed want to tally over cells *not* filled with a material,
that each of those cells must have a unique ID (recall that for lattices, a cell with the
same ID is repeated multipled times, with each repeated cell given a different *instance*).
This just requires us to construct the geometry slightly differently.

!listing /tutorials/trisos/make_openmc_model.py

The OpenMC geometry, colored by material, is shown below. We used OpenMC's
[random packing features](https://docs.openmc.org/en/stable/examples/triso.html)
for generating a random packing of [!ac](TRISO) particles. We generate a *single* random
realization, and then repeat it for each pebble. We also apply a lattice on top of the
[!ac](TRISO) particles *and* a second lattice on top of the pebbles to speed up the
geometric search. This lattice differs from the notion of a lattice used in
[Tutorial 2A](pincell1.md), and moreso refers to a superimposed search grid. For a three
pebble problem, adding a search grid to the pebbles has an insignificant effect, but should
certainly be included for larger problems.

!media pbr_openmc.png
  id=pbr_openmc
  caption=OpenMC geometry (colored by material) for a stack of three pebbles
  style=width:20%;margin-left:auto;margin-right:auto




!bibtex bibliography
