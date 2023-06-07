# UO$_2$ Pebbles

In this tutorial, you will learn how to:

- Couple OpenMC via temperature and heat source feedback to MOOSE for pebbles
- Establish coupling between OpenMC and MOOSE for nested universe OpenMC models
- Couple OpenMC solves in units of centimeters with MOOSE solves in units of meters
- Repeat the same mesh tally several times throughout the OpenMC domain

To access this tutorial,

```
cd cardinal/tutorials/pebbles
```

!alert! note title=Computing Needs
No special computing needs are required for this tutorial.
For testing purposes, you may choose to decrease the number of particles to
solve faster.
!alert-end!

## Geometry and Computational Models

The geometry and
computational model for this case consists of a vertical "stack"
of three solid UO$_2$ pebbles, each of 1.5 cm radius. The pebble centers
are located at $(0, 0, 2)$ cm, $(0, 0, 6)$ cm, and $(0, 0, 10)$ cm.
FLiBe coolant occupies the space around the pebbles. Heat is produced in the
pebbles; we assume the total power is 1500 W.

While "pebbles" in nuclear applications typically refer to
[!ac](TRISO) fuel geometries, this tutorial only considers homogeneous
pebbles so as to serve as a stepping stone to modeling heterogeneous
[!ac](TRISO) geometries in [Tutorial 6C](gas_compact.md). Here, we first emphasize
important features of the OpenMC wrapping having to do with lattices
and unstructured mesh tallies.

### Heat Conduction Model

!include steady_hc.md

The solid mesh is shown in [solid_mesh]. The pebble surface is sideset 0.
The MOOSE solid problem is set up with a length unit of meters,
which is convenient
for heat transfer applications because
material properties for thermal-fluids physics are almost always given in SI units. Further,
many MOOSE physics modules inherently assume SI units, such as the
[fluid property module](https://mooseframework.inl.gov/modules/fluid_properties/index.html)
and [solid property module](https://mooseframework.inl.gov/modules/solid_properties/index.html).

!media pebble_mesh.png
  id=solid_mesh
  caption=Mesh for solid domain
  style=width:15%;margin-left:auto;margin-right:auto

Because heat transfer and fluid flow in the FLiBe is not modeled in this example,
heat removal by the fluid is approximated by setting the outer surface of the
pebble to a convection boundary condition,

\begin{equation}
\label{eq:1}
q^{''}=h\left(T-T_\infty\right)
\end{equation}

where $h=1000$ W/m$^2$/K and $T_\infty$ is set to a function linearly ranging
from 650&deg;C at $z=0$ cm to 750&deg;C at $z=10$ cm. In this example, the MOOSE
heat conduction module will be run first. The initial solid temperature is set to
650&deg;C and the heat source to zero.

### OpenMC Model

The OpenMC model is built using [!ac](CSG); we will leverage the *lattice*
feature in OpenMC to repeat a universe in a structured manner throughout the domain.
Here, the universe consists of a single pebble and surrounding FLiBe, which is repeated
three times throughout the geometry. The overall domain is enclosed in a box with
$x$-$y$ width of 4$\times$4 cm and height of 12 cm. All boundaries of this box are reflecting.

OpenMC's Python [!ac](API) is used to create the pebbles model with the script shown below.
First, we define materials for the various regions and create the geometry.
We create a universe consisting of a single pebble, and then repeat that universe
three times in a lattice. Because we have one cell per pebble, each pebble will
receive a single temperature from MOOSE. The solid cell level is 1
because the pebble lattice is nested once
with respect to the highest level, and we want to apply
feedback to each pebble individually. If we were to specify level zero in the geometry, we would apply feedback
to the `main_cell`, which would apply temperature feedback across the problem globally.
The OpenMC geometry as produced via plots is shown in [mc_geom].

!listing /tutorials/pebbles/make_openmc_model.py

!media pebble_openmc.png
  id=mc_geom
  caption=OpenMC geometry (colored by cell ID)
  style=width:15%;margin-left:auto;margin-right:auto

As you can see, there
are only two unique cell *IDs*. This problem is built using *distributed cells*, meaning a cell
with the same ID appears multiple times in the geometry; each time the cell is repeated, we assign
a new *instance* to that cell. So, the three pebbles are represented as:

- Cell ID 1, instance 0
- Cell ID 1, instance 1
- Cell ID 1, instance 2

and the FLiBe region is represented as:

- Cell ID 2, instance 0
- Cell ID 2, instance 1
- Cell ID 2, instance 2

Because OpenMC runs after the MOOSE heat conduction module, initial conditions are
only required for the FLiBe temperature, which is set to 650&deg;C. Because there is
no density feedback in this example, the densities initially imposed in the OpenMC
model remain fixed at the values set in the OpenMC input files.

To create the XML files required to run OpenMC, run the script:

```
python make_openmc_model.py
```

## Multiphysics Coupling

In this section, OpenMC and MOOSE are coupled for heat source and temperature feedback for
the solid regions of a stack of three pebbles.
The following sub-sections describe these files.

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
$(0, 0, 0.02)$ cm in `openmc.i` (when we actually want the point to map to
$(0, 0, 2)$ cm).

Next, we define auxiliary variables that will help us visualize the cell IDs
([CellIDAux](/auxkernels/CellIDAux.md)),
cell instances ([CellInstanceAux](/auxkernels/CellInstanceAux.md)),
and cell temperatures ([CellTemperatureAux](/auxkernels/CellTemperatureAux.md))
as they map to the `[Mesh]`.

!listing /tutorials/pebbles/openmc.i
  start=AuxVariables
  end=Problem

The `[Problem]`
block is then used to specify the OpenMC wrapping. We define a total power of
1500 W, and indicate that we'd like to add tallies on block 0, which corresponds to the pebbles.
The cell tally setup in Cardinal will then automatically add a tally for each unique
cell ID+instance combination.
Because the repeated pebble cells we'd like to tally
are repeated in the lattice nested one level below the root universe, we set the `solid_cell_level = 1`.

!listing /tutorials/pebbles/openmc.i
  block=Problem

The `scaling` parameter is used to indicate a multiplicative factor that should be
applied to the `[Mesh]` in order to get to units of centimeters.
Because the `[Mesh]` is in units of meters, we set `scaling = 100.0`.
This scaling factor is
applied within the `OpenMCCellAverageProblem::findCell` routine that maps MOOSE elements to OpenMC cells -
no actual changes are made to the mesh in the `[Mesh]` block.
The scaling is also applied to ensure that the heat source is on the correct
per-unit-volume scale that is expected by the `[Mesh]`.

Finally, we set a transient executioner, specify an Exodus output, and
define several postprocessors.

!listing /tutorials/pebbles/openmc.i
  start=Executioner

## Execution and Postprocessing

To run the coupled calculation,

```
mpiexec -np 2 cardinal-opt -i solid.i --n-threads=2
```

This will run both MOOSE and OpenMC with 8 MPI processes and 2 OpenMP threads per rank.
When the simulation has completed, you will have created a number of different output files:

- `solid_out.e`, an Exodus file with the solid mesh and solution
- `solid_out_openmc0.e`, an Exodus file with the OpenMC solution and the data
  that was ultimately transferred in/out of OpenMC

First, let's examine how the mapping between OpenMC and MOOSE was established.
When we run with `verbose = true`, you will see the following mapping information displayed:

```
 ===================>     MAPPING FROM OPENMC TO MOOSE     <===================

          Solid:  # elems in 'solid_blocks' each cell maps to
          Fluid:  # elems in 'fluid_blocks' each cell maps to
          Other:  # uncoupled elems each cell maps to
     Mapped Vol:  volume of MOOSE elems each cell maps to
     Actual Vol:  OpenMC cell volume (computed with 'volume_calculation')

--------------------------------------------------------------------------
|         Cell         | Solid | Fluid | Other | Mapped Vol | Actual Vol |
--------------------------------------------------------------------------
| 1, instance 0 (of 3) |   448 |     0 |     0 | 1.322e-05  |            |
| 1, instance 1 (of 3) |   448 |     0 |     0 | 1.322e-05  |            |
| 1, instance 2 (of 3) |   448 |     0 |     0 | 1.322e-05  |            |
--------------------------------------------------------------------------
```

The three cells representing the pebbles are mapped to the corresponding MOOSE elements for each pebble.
Shown below is the heat source computed by OpenMC,
along with the temperature computed by MOOSE (before and after being averaged and sent to OpenMC cells).
The temperature is shown on a cut plane through the centers of the pebbles;
because one temperature is set for each pebble, the temperature in OpenMC for
each pebble is an average over the pebble.

!media pbr_solution.png
  id=pbr_solution
  caption=Heat source and temperature computed by a coupled OpenMC-MOOSE model of pebbles

You can confirm that the `scaling` factor was applied correctly by comparing the
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
a mesh of 300,000 pebbles that is ~150 GB).

The files for this stage of the coupling are the
`solid_um.i` and `openmc_um.i` inputs in the `tutorials/pebbles` directory.
For the solid, we simply need to swap out the sub-application to point to a different OpenMC
input file.

!listing /tutorials/pebbles/solid_um.i
  block=MultiApps

We also use a finer solid pebble heat conduction mesh to provide an example of the
case where the OpenMC `[Mesh]` block differs from the mesh used in the coupled MOOSE
application. To do this, we increase `nr` to add more radial discretization.

!listing /tutorials/pebbles/solid_um.i
  block=Mesh

For the OpenMC wrapping, the only changes required are
that we set the type of tally to `mesh`, provide a mesh template with the mesh, and specify the
translations to apply to replicate the mesh at the desired end positions in OpenMC's domain.
For the mesh tally, let's create a mesh for a single pebble using MOOSE's mesh generators. We simply
need to run the `mesh.i` file in `--mesh-only` mode:

!listing /tutorials/pebbles/mesh.i

```
cardinal-opt -i mesh.i --mesh-only
```

which will create a mesh file named `mesh_in.e`. We then list that mesh as the
`mesh_template` in the `[Problem]` block.

!listing /tutorials/pebbles/openmc_um.i
  block=Problem

Note that the mesh template and mesh translations must be in the same
units as the `[Mesh]` block.
In addition, because our sphere mesh does not perfectly preserve the volume of the sphere
cells, we set `normalize_by_global_tally` to false so that we normalize only
by the sum of the mesh tally. Otherwise, we would miss a small amount of power produced
within the spheres, but slightly outside the faceted surface of the sphere mesh. Setting this parameter to false ensures that the tally normalization is correct in that the heat sources are normalized by a tally sum over the same tally domain in the OpenMC model.

To run this input,

```
mpiexec -np 2 cardinal-opt -i solid_um.i --n-threads=2
```

[mesh3] shows the heat source computed by OpenMC, the heat source applied in MOOSE,
the temperature computed by MOOSE, and the temperature applied in OpenMC. The MOOSE
transfers handle the different meshes seamlessly. The mesh translation feature allows
a single pebble mesh to be translated to three individual positions in the OpenMC tally.
Note that the heat sources on the OpenMC `[Mesh]` and on the MOOSE heat conduction mesh
are shown on a different color scale - the volumetric power density on the MOOSE mesh is
lower than that on the OpenMC mesh because the MOOSE mesh has a greater volume
(since there is a better approximation of the sphere volume). The conservative
[MultiAppNearestNodeTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppNearestNodeTransfer.html)
conserves total integrated power.

!media pebble_hs_mesh3.png
  id=mesh3
  caption=Temperature and heat source distributions shown on the different MOOSE and OpenMC meshes. The temperatures are shown on a slice through the centers of the pebbles.

Finally, recall that adding unstructured mesh tallies does not affect the resolution of temperature and
density feedback sent to OpenMC - this resolution is controlled by the cell definitions
when constructing the OpenMC input. Therefore, each pebble still receives a single temperature
value from MOOSE because each pebble is represented as a single cell.
