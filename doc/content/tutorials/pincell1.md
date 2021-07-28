# Tutorial 6A: LWR Pincell

In this tutorial, you will learn how to:

- Couple OpenMC via temperature and heat source feedback to MOOSE
- Control the resolution of physics feedback received by OpenMC
- Tally an OpenMC heat source on an unstructured mesh

This tutorial describes how to use Cardinal to perform temperature and heat
source coupling of OpenMC to MOOSE for a [!ac](LWR) pincell. While the entire
domain is modelled by OpenMC, the MOOSE domain only consists of the solid regions.
After providing the foundation in this tutorial,
in [Tutorial 7](openmc_fluid.md) we will describe how to introduce both
temperature and density feedback from MOOSE to OpenMC. Finally, in
[Tutorial 9](coupled.md), we will combine the content of the previous tutorials
to couple NekRS, OpenMC, and MOOSE heat conduction for fully-coupled multiphysics.

## Particle Transport Coupling

At a high level, Cardinal's wrapping of OpenMC consists of two major stages - first, establishing
the mapping between OpenMC's geometry and the [MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html)
with which OpenMC communicates. This stage consists of:

- Mapping the elements in a [MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html)
  to OpenMC cells. The mapping is quite flexible, and there are no requirements on geometry alignment.
  MOOSE mesh elements are mapped to OpenMC cells by identifying the OpenMC cell that resides at the
  MOOSE element's centroid.
- Identifying which MOOSE mesh blocks are "solid" and which are "fluid." The solid blocks will then
  exchange temperature with OpenMC, while the fluid blocks will exchange both temperature and fluid
  density with OpenMC.
- If using cell tallies, identifying which MOOSE blocks should be tallied - tallies are then added to all OpenMC cells that
  correspond to those elements.

The second stage of the wrapping encompasses the actual multiphysics solve:

- Adding [MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
  to represent OpenMC's heat source. In other words, if OpenMC stores the fission heating tally
  as a `std::vector<double>`, with each entry corresponding to a [!ac](CSG) cell or unstructured
  mesh element (for unstructured mesh tallies), then a [MooseVariable](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
  is created that represents the same data, but mapped to the [MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html).
- Writing multiphysics feedback fields in/out of OpenMC's internal cell and material representations.
  So, if OpenMC represents cell temperature as `std::vector<double>`, this involves reading
  from a [MooseVariable](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
  representing temperature (which can be transferred with any of MOOSE's transfers to the
  NekRS wrapping) and writing into OpenMC's internval vectors. A similar process occurs for
  density feedback in OpenMC.

Cardinal developers have an intimate knowledge of how OpenMC stores its tally results
and cell temperatures and densities, so this entire process is automated for you! OpenMC
can communicate with any other MOOSE application via the [MultiApp](https://mooseframework.inl.gov/syntax/MultiApps/index.html)
and [Transfer](https://mooseframework.inl.gov/syntax/Transfers/index.html)
systems in MOOSE. The same wrapping can be used for temperature, density, and heat source
feedback with *any* MOOSE application that can compute temperature and density -
including the NekRS wrapping in Cardinal. The MOOSE-wrapped version of OpenMC interacts
with the MOOSE framework in a similar manner as natively-developed MOOSE applications,
so the agnostic formulations of the [MultiApps](https://mooseframework.inl.gov/syntax/MultiApps/index.html)
and [Transfers](https://mooseframework.inl.gov/syntax/Transfers/index.html) can be used
to equally set a heat source in Pronghorn or NekRS, and extract temperature and density from
BISON, NekRS, and Pronghorn.

## Geometry and Computational Model

This section describes the geometry for a [!ac](LWR) pincell; the relevant dimensions
are summarized in [table1]. The pincell consists of a UO$_2$ pincell within a Zircaloy
cladding; a helium gap separates the fuel from the cladding. Borated water
is present outside the cladding.

!table id=table1 caption=Geometric and operating conditions for a [!ac](LWR) pincell
| Parameter | Value |
| :- | :- |
| Pellet outer radius | 0.39218 cm |
| Clad inner radius | 0.40005 cm |
| Clad outer radius | 0.45720 cm |
| Pin pitch | 1.25984 cm |
| Height | 300 cm |

Heat is produced in the pellet region and transfers by radiation and conduction across
the pellet-clad gap to the cladding. A total core power of 3000 MWth is assumed uniformly
distributed among 273 fuel bundles, each with 17$\times$17 pins. The tallies from OpenMC
will therefore be normalized according to a pin-wise power of

\begin{equation}
\label{eq:1}
q_{pin}=\frac{3000\text{\ MWth}}{n_bn_p}
\end{equation}

where $n_b=273$ is the number of fuel bundles and $n_p=289$ is the number of pins
per bundle.

## Boundary Conditions

This section describes the boundary conditions imposed on the Monte Carlo particle transport model and
the MOOSE heat conduction model.

### Neutronics Boundary Conditions

For the neutronics physics, the top and bottom of the pincell are assumed vacuum
conditions. For the four lateral faces of the pincell, reflective boundaries are used.

### Solid Boundary Conditions

Because heat transfer and fluid flow in the borated water is not modeled in this example,
we approximate the effect of heat removal by the fluid by setting
the outer surface of the cladding to a convection boundary
condition,

\begin{equation}
\label{eq:2}
q^{''}=h\left(T-T_\infty\right)
\end{equation}

where $h=1000$ W/m$^2$/K and $T_\infty=280$&deg;C. The top and bottom of the solid pincell
are assumed insulated.

The gap region between the pellet and the cladding is unmeshed, and a quadrature-based
thermal contact model is applied based on the sum of thermal conduction and thermal radiation
(across a transparent medium).
For a paired set of boundaries,
each quadrature point on boundary A is paired with the nearest quadrature point on boundary B.
Then, the sum of the radiation and conduction heat fluxes imposed between pairs of
quadrature points is

\begin{equation}
\label{eq:3}
q^{''}=\sigma\frac{T^4-T_{gap}^4}{\frac{1}{\sigma_A}+\frac{1}{\sigma_B}-1}+\frac{T-T_{gap}}{r_{th}}
\end{equation}

where $\sigma$ is the Stefan-Boltzmann constant, $T$ is the temperature at a quadrature
point, $T_{gap}$ is the temperature of the nearest quadrature point across the gap,
$\sigma_A$ and $\sigma_B$ are emissivities of boundaries A and B, respectively, and
$r_{th}$ is the conduction resistance. For cylindrical geometries, the conduction
resistance is given as

\begin{equation}
\label{eq:4}
r_{th}=\frac{ln{\left(\frac{r_2}{r_1}\right)}}{2\pi L k}
\end{equation}

where $r_2>r_1$ are the radial coordinates associated with the outer and inner radii
of the cylindrical annulus, $L$ is the height of the annulus, and $k$ is the
thermal conductivity of the annulus material.

## Initial Conditions

The initial temperature is 280&deg;C, while the initial heat source in solid domain is zero.

## Meshing

This section describes the mesh used for the solid domain. MOOSE
[MeshGenerators](https://mooseframework.inl.gov/syntax/Mesh/index.html) are used to construct
the solid mesh. [solid_mesh] shows the mesh with block IDs and sidesets.
Vecause this mesh is generated using
the [MeshGenerator](https://mooseframework.inl.gov/syntax/Mesh/)
system in MOOSE, there is not a mesh file that describes the solid mesh.
You can view the solid mesh either by running the simulation (and viewing the mesh
on which the results are displayed), or simply by running the solid input file in
mesh generation mode:

!listing
$ cardinal-opt -i solid.i --mesh-only

!media pincell_solid_mesh.png
  id=solid_mesh
  caption=Mesh for the solid portions of a [!ac](LWR) pincell

## CSG Geometry

This section describes the [!ac](CSG) model setup in OpenMC. In the near future,
OpenMC will have unstructured mesh tracking capabilities - but until that is available,
OpenMC geometries all use the [!ac](CSG) geometry approach, where cells are created
from half-spaces of various common surfaces.
Because this is a Cardinal
tutorial, we assume you have some basic familiarity with OpenMC, so we only discuss the
portions of the model setup relevant to multiphysics feedback. When creating the OpenMC
geometry, there are two aspects that you must pay attention to when using Cardinal -

- The resolution of the temperature (and density, for fluid feedback) feedback to impose in OpenMC
- The "level" of the cells with which you want to perform feedback

The temperature in OpenMC is stored on the [!ac](CSG) cells. One constant temperature
can be set for each cell. Therefore, the resolution of the temperature feedback
received from MOOSE is determined during the OpenMC model setup. Each
OpenMC cell will receive a unique temperature, so the number of OpenMC cells dictates
this feedback resolution.

The second consideration is slightly more subtle, but allows great flexibility for
imposing multiphysics feedback for very heterogeneous geometries, such as
[!ac](TRISO) pebbles. The "level" of a [!ac](CSG) cell refers to the number of
nested universes (relative to the root universe) at which you would like to impose
feedback. If you construct your geometry without *filling* any OpenMC cells with
other universes, then all your cells are at level zero - i.e. the highest level in
the geometry. But if your model contains lattices, the level that you want to perform
multiphysics coupling on is most likely not the highest level in the geometry. For instance,
[!ac](LWR) cores are comprised of hundreds of assemblies. Your approach to creating the
geometry would probably be to make a single fuel assembly universe, then repeat that
lattice several times throughout the geometry. If your assembly universe wasn't itself
filled with any universes, then all your cells of interest are actually at level 1.
Cardinal allows the flexibility to set the temperature/density for all cells *contained*
in a particular cell, if desired.
We will illustrate this lattice concept with a set of [!ac](TRISO) pebbles
in [Tutorial 2B](triso.md).

!alert note
We *highly* recommend running Cardinal with `verbose = true` when setting up
your OpenMC coupling. This setting will display the mapping of OpenMC cells to
MOOSE elements and should help provide a grasp on the "level" concept.

OpenMC's Python [!ac](API)
is used to create the model with the script shown below. First, we define
materials for the various regions and create the geometry. With respect to the
first important consideration when setting up the OpenMC model - we manually add
12 cells to receive solid temperature feedback by dividing the entire axial
height by 13 axial planes.

!alert note
The particular choice of axial cells has *no* relationship to the solid mesh.
That is, MOOSE elements can span more than one OpenMC cell; when mapping by centroid,
however, each MOOSE element will then be associated with one OpenMC cell (which may be
imperfect, but represents a form of discretization error in the MOOSE element refinement).

!listing /tutorials/lwr_solid/make_openmc_model.py

And with respect to the second important consideration,
because we are not filling any universes with other universes or lattices,
all of the cells in this problem are at the highest level of the geometry -
i.e. the root universe. Later in the OpenMC wrapping, we will need to provide
this information, so we simply make a note of it here.

!alert note
For obtaining *temperature* feedback in OpenMC, we simply need to create unique
cells to receive the temperature. We do not need to create unique materials, for
instance.

The OpenMC geometry as produced via plots is shown below.

!media pincell_openmc.png
  id=pincell_openmc
  caption=OpenMC [!ac](CSG) geometry (colored by cell ID) shown along the $x$-$y$ and $x$-$z$ planes
  style=width:60%;margin-left:auto;margin-right:auto

## Multiphysics Coupling
  id=coupling

In this section, OpenMC and MOOSE are coupled for heat source and temperature feedback
for the solid regions of a [!ac](LWR) pincell. All input files are present in the
`tutorials/lwr_solid` directory. The following sub-sections describe these files.

### Solid Input Files

The solid phase is solved with the MOOSE heat conduction module, and is
described in the `solid.i` input. The solid mesh is set up using various
mesh generators.

!listing /tutorials/lwr_solid/solid.i
  end=Variables

!alert note
OpenMC *always* has geometries set up in length units of centimeters. However,
MOOSE is dimension-agnostic, and the same physics model can be set up in any
length unit provided proper scalings are applied to material properties, source terms,
and the mesh. In this example, we set up the solid input file in length units of centimeters.
In [Tutorial 6B](triso.md), we will set up the MOOSE input file in a different set
of units for illustration.

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

!alert note
The heat source does not need to be of type `CONSTANT MONOMIAL`, but we
typically use this basis because the heat source in OpenMC is
`CONSTANT MONOMIAL` (one value per cell/element). The heat source
variable defined later in `openmc.i` is `CONSTANT MONOMIAL`, so using
the same variable basis can let us use the
[MultiAppCopyTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppCopyTransfer.html)
in the event that the meshes are exactly the same (they don't need to
be - we just typically use the same basis out of habit).

The governing equation solved by MOOSE is specified in the `Kernels` block with the
[HeatConduction](https://mooseframework.inl.gov/source/kernels/HeatConduction.html)
and [CoupledForce](https://mooseframework.inl.gov/source/kernels/CoupledForce.html) kernels,
with the heat source provided by OpenMC via the receiver `heat_source` auxiliary variable,
giving the equation $-\nabla\cdot(k\nabla T)=\dot{q}$.

!listing /tutorials/lwr_solid/solid.i
  block=Kernels

We also set thermal conductivity values in the pellet and clad.

!listing /tutorials/lwr_solid/solid.i
  block=Materials

Next, the boundary conditions on the solid are applied, including the thermal
contact model between the pellet and the clad.

!listing /tutorials/lwr_solid/solid.i
  start=BCs
  end=Materials

In this example, the overall calculation workflow is as follows:

- Run MOOSE heat conduction with a given power distribution from OpenMC.
- Send temperature to OpenMC by modifying the temperature of OpenMC cells.
  A volume average is performed over all the MOOSE elements that mapped to
  a particular OpenMC cell.
- Run OpenMC with an updated temperature distribution
- Extract the kappa-fission distribution (the recoverable fission energy)
  computed by OpenMC and map in the opposite direction from OpenMC cells
  to all the MOOSE elements that mapped to each cell.

The above sequence is repeated until convergence of the coupled domain.
The [MultiApps](https://mooseframework.inl.gov/syntax/MultiApps/index.html)
and [Transfers](https://mooseframework.inl.gov/syntax/Transfers/index.html)
blocks then describe the interaction between Cardinal and MOOSE. The MOOSE
heat conduction application is here run as the master application, with the
OpenMC wrapping run as the sub-application. We specify that MOOSE will run
first on each time step.

Two transfers are required to couple OpenMC and MOOSE for heat source and
temperature feedback; the first is a transfer of heat source from Cardinal
to MOOSE. The second is transfer of temperature from MOOSE to Cardinal.

!listing /tutorials/lwr_solid/solid.i
  start=MultiApps

Finally, we run for ten time steps. Because we did not specify sub-cycling
in the `MultiApps` block, this means that OpenMC will run for exactly the
same number of time steps (but the actual time step size used by the OpenMC
wrapping is of no consequence because OpenMC is run in k-eigenvalue mode).

!listing /tutorials/lwr_solid/solid.i
  start=Executioner
  end=MultiApps

### Neutronics Input Files

The neutronics physics is solved over the entire domain using OpenMC.
The OpenMC wrapping is described in the `openmc.i` input file. Although OpenMC
does not track particles on a mesh, we begin by defining a mesh on which OpenMC
will receive temperature from the coupled MOOSE application, and on which OpenMC
will write the fission heat source. In this example, we use exactly the same solid
mesh as the coupled MOOSE application, but this is not necessary.

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
as "solid" (and therefore send temperatures into OpenMC) with the `solid_blocks` parameter.
Here, we specify temperature feedback for the pellet (blocks 2 and 3) and the cladding
(block 1). During the initialization, [OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md)
will automatically map from MOOSE elements to OpenMC cells, and store which MOOSE elements
are "solid." Then when temperature is sent into OpenMC, that mapping is used to compute
a volume-average temperature to apply to each OpenMC cell. While we use nomenclature
like `solid_blocks`, please note that these blocks don't need to actually represent
*solid* - we only use this parameter to note where OpenMC should receive temperature
feedback, but *not* density feedback (which would require moving the cell boundaries
for deforming solids).

This problem uses cell tallies, as indicated by the `tally_type` parameter.
The `tally_blocks` is then used to indicate which OpenMC cells to add tallies to
(as inferred from the mapping from MOOSE elements to OpenMC cells). If not specified,
we add tallies to all OpenMC cells - but for this problem, we already know that the
cladding doesn't have any fissile material, so we can save some effort with the
tallies by skipping tallies in those regions by setting `tally_blocks = '2 3'`.
[OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md) will then
automatically add the necessary tallies - as you saw earlier, there are no
tallies added in the creation of the XML OpenMC input files.

Finally, we specify the level in the geometry on which the solid cells
lie. Because we don't have any lattices or filled universes in our OpenMC model,
we set the level to zero.

Next, we add a series of auxiliary variables *just* for solution visualization -
these are not requried for coupling, and are entirely optional. To help with understanding
how the OpenMC model maps to the mesh in the `[Mesh]` block, we add auxiliary
variables to render OpenMC's cell ID ([CellIDAux](/auxkernels/CellIDAux.md))
and cell temperature ([CellTemperatureAux](/auxkernels/CellTemperatureAux.md)) on
the `[Mesh]`.

Next, we specify an executioner and output settings. Even though OpenMC technically
performs a criticality calculation (with no time dependence), we use the transient
executioner so that *if* we wanted to run OpenMC more times than the coupled
master application via subcycling, we would have a way to control that.

!listing /tutorials/lwr_solid/openmc.i
  start=Executioner
  end=Postprocessors

The OpenMC tally results are normalized by the volumes of the elements in the
`[Mesh]`. MOOSE by default chooses the quadrature rule of an application
according to the order of the nonlinear variables solved by that application.
Here, because we don't have any nonlinear variables solved by *MOOSE* (of course
we have tallies "solved" by OpenMC), the framework chooses a quadrature rule lower
than what is used by default in the `solid.i` input. This means that, even if
we normalize OpenMC's power correctly in `openmc.i`, if we integrated the
heat source received in `solid.i`, we wouldn't match the heat source integrated
in `openmc.i` due to different quadrature rules. We simply explicitly set the
quadrature rule here to match what is used by `solid.i` (which doesn't appear
in the input file because it's the default).

Finally, we add a postprocessor to show that the heat source computed by OpenMC
for diagnostic purposes.

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
  [density] # only present if fluid_blocks was provided
    family = MONOMIAL
    order = CONSTANT
  []
[]

In addition to the wrapping of OpenMC in the `openmc.i` input file, we
need to create the XML input files used to run OpenMC from the Python script shown earlier.
You can run the script to generate the input files with:

```
$ python make_openmc_model.py
```

There is one additional important aspect of the OpenMC model setup that must be
discussed - how temperatures are applied to the cross sections and what temperatures are loaded
for the cross section data. The [temperature method](https://docs.openmc.org/en/latest/io_formats/settings.html#temperature-method-element)
 element is used to indicate whether a nuclide temperature should be interpolated
between the cross section data sets at the two closest temperatures (`interpolation`)
or whether the nearest data set should be used (`nearest`). We recommend using the
`interpolation` method unless your cross section data does not have very wide gaps
between data sets. Many of Cardinal's regression tests use the `nearest` method, though,
since this option is less sensitive to floating point differences.

When OpenMC is initialized, cross section data is only loaded for a range of temperatures;
this range is specified with the [temperature range](https://docs.openmc.org/en/latest/io_formats/settings.html#temperature-range-element)
element. You should set the range to be larger than the temperature range you
expect in your coupled multiphysics problem.

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
cell  1, instance  0 (of  1):   600 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  10.6939
cell  3, instance  0 (of  1):   180 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  3.40634
cell  5, instance  0 (of  1):   800 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  14.2585
cell  7, instance  0 (of  1):   240 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  4.54179
cell  9, instance  0 (of  1):   600 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  10.6939
cell 11, instance  0 (of  1):   180 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  3.40634
cell 13, instance  0 (of  1):   600 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  10.6939
cell 15, instance  0 (of  1):   180 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  3.40634
cell 17, instance  0 (of  1):   800 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  14.2585
cell 19, instance  0 (of  1):   240 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  4.54179
cell 21, instance  0 (of  1):   600 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  10.6939
cell 23, instance  0 (of  1):   180 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  3.40634
cell 25, instance  0 (of  1):   600 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  10.6939
cell 27, instance  0 (of  1):   180 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  3.40634
cell 29, instance  0 (of  1):   800 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  14.2585
cell 31, instance  0 (of  1):   240 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  4.54179
cell 33, instance  0 (of  1):   600 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  10.6939
cell 35, instance  0 (of  1):   180 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  3.40634
cell 37, instance  0 (of  1):   600 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  10.6939
cell 39, instance  0 (of  1):   180 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  3.40634
cell 41, instance  0 (of  1):   800 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  14.2585
cell 43, instance  0 (of  1):   240 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  4.54179
cell 45, instance  0 (of  1):   600 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  10.6939
cell 47, instance  0 (of  1):   180 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  3.40634
```

This shows that a total of 24 OpenMC cells mapped to the MOOSE blocks (blocks 1, 2, and 3 -
because that's what we specified for `solid_blocks`). This message also shows the
volume that each OpenMC cell maps to. Because there are no distributed cells in this
problem, each cell only has a single instance.

!alert note
Because we used 12 cells in the axial direction to represent the feedback in OpenMC,
and because we only used 40 layers to construct the solid mesh in the `[Mesh]`
block, there is not a perfect correspondence between OpenMC cells and mesh elements.
Even though all of the cells in the UO$_2$ region in the OpenMC geometry are the
same size, some map to larger regions of the mesh because 40 is not evenly divisible
by 12. This is intentional behavior to allow flexible geometry mappings, but you can
of course create the OpenMC model and MOOSE mesh with more attention to how the data
is mapped.

Shown below is the heat source computed by OpenMC (units of W/cm$^3$)
and mapped to the MOOSE mesh;
the block corresponding to the cladding is not shown.
To the right is shown the heat source mapped along a line down the centerline of
the rod. By running more particles and increasing the number of axial cells in the
OpenMC models, you will observe the parabolic power distribution typical of
[!ac](LWR) geometries.

!media lwr_heat_source.png
  id=lwr_heat_source
  caption=Heat source computed by OpenMC

[lwr_solid_temp] shows the temperature computed by the MOOSE heat conduction
module, while [lwr_solid_temp_mc] shows the temperature actually set in the OpenMC
cells mapped to the MOOSE elements. Because a single cell was used to represent
the cladding and fuel (at each axial layer), only one temperature is shown for the
fuel and clad regions in the `cell_temperature` auxiliary variable. The temperatures
set in OpenMC are volume averages of the temperature computed by MOOSE, i.e. the
temperature shown in [lwr_solid_temp]. The same color scale is used in
[lwr_solid_temp] and [lwr_solid_temp_mc].

!media lwr_solid_temp.png
  id=lwr_solid_temp
  caption=Temperature computed by MOOSE, on the bottom of the pincell
  style=width:55%;margin-left:auto;margin-right:auto

!media lwr_solid_temp_openmc.png
  id=lwr_solid_temp_mc
  caption=Temperature set in OpenMC cells (shown in terms of the MOOSE mesh elements that map to each cell), on the bottom of the pincell
  style=width:55%;margin-left:auto;margin-right:auto

## Adding Mesh Tallies

As the next part of this tutorial, we will repeat the simulation but replace the cell
tallies with unstructured mesh tallies. That is, instead of setting `tally_blocks` and
providing the MOOSE blocks to which the corresponding OpenMC cells should have tallies added,
we will simply tally on an unstructured mesh. The inputs for this problem are largely the
same as in [#coupling]; the files are now `solid_um.i` and `openmc_um.i`.
 or the solid, we simply need to swap out the sub-application to
point to a different input file.

!listing /tutorials/lwr_solid/solid_um.i
  block=MultiApps

Then, in `openmc_um.i`, we make small modifications to the settings for the
[OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md). We indicate that
`tally_type = mesh` and provide the unstructured mesh that we want to tally as a file
with the `mesh_template` parameter.

!listing /tutorials/lwr_solid/openmc_um.i
  block=Problem

By default, Cardinal will normalize the OpenMC fission energy tally according
to a global tally over the entire OpenMC problem. When using mesh tallies on
curvilinear surfaces, many unstructured meshes cannot perfectly represent the
domain. In this problem, for instance, the faceted nature of the pincell mesh
means that a (small) amount of the fission energy is omitted in the tally because
some regions of the OpenMC [!ac](CSG) cell are *outside* any of the tally mesh
elements. In order to still obtain a total power given by the `power` parameter,
we therefore change how the tallies are normalized. Instead of normalizing by a
problem-wide tally (which includes the regions of the pellet that are outside
the unstructured mesh), we normalized instead by the sum of the mesh tally itself
by setting `normalize_by_global_tally = false`. This ensures that the power we
specify will be obtained when normalizing the OpenMC tally.

Even though we set up the mesh with
MOOSE's mesh generator system, we output the mesh template, `pincell.e` by running
MOOSE in mesh-generation mode, which will output the mesh as an Exodus file.

```
$ cardinal-opt -i solid_um.i --mesh-only
$ mv solid_um_in.e pincell.e
```

Note that because the mesh in the `[Mesh]` block contains elements that correspond
to the cladding, we will technically be tallying in cladding regions, even though
there isn't a heat source there.

!alert warning
There are several important limitations in the current implementation of mesh tallies
in Cardinal - these will be relaxed in the future, but you must be aware of them with the
current state of the repository. First, if the mesh provided by the `mesh_template`
has $N$ elements, those elements must
*exactly* match the first $N$ elements in the mesh in the `[Mesh]` block. The reason for this limitation is that
the heat source tally is simply written to the corresponding mesh element in the
`[Mesh]` by element index (as opposed to doing a nearest-element search). If the mesh in the
`[Mesh]` block contains both solid and fluid elements, for instance, and you only want to
tally on an unstructured mesh in the solid, all the solid elements in the `[Mesh]` should
appear first in the total combined mesh. You can use a [CombinerGenerator](https://mooseframework.inl.gov/source/meshgenerators/CombinerGenerator.html)
to achieve this if your fluid and solid meshes are saved in separate files
or if you use separate mesh generators for the phases. We have
checks in place to make sure you don't inadvertently bypass this requirement.
Second,
the `mesh_template` *must* be in units of centimeters; this is relevant when
solving the MOOSE application in different units, like you will learn in
[Tutorial 2B](triso.md).

To run the input using mesh tallies, enter the following at the command line.

```
$ mpiexec -np 8 cardinal-opt -i solid_um.i --n-threads=2
```

[mesh_hs] shows the unstructured mesh heat source computed by OpenMC. Due to the
very small number of particles used, there are relatively few scores to each mesh element,
so the uncertainty of these results is high.

!media mesh_hs.png
  id=mesh_hs
  caption=Unstructured mesh heat source computed by OpenMC, shown on the bottom of the pincell, with 1000 particles per batch, 10 inactive batches, and 90 active batches
  style=width:55%;margin-left:auto;margin-right:auto

If you edit the `settings.xml` file and increase the number of particles per batch
to 500000 and also increase the number of inactive and active batches,
the mesh tally is computed as shown in [mesh_hs2]. By reducing the tally uncertainty,
we begin to see the "rim effect" common in [!ac](LWR) fuels, where the highest power
is observed near the very edge of the fuel pellet. In [mesh_hs2], the clad region is not shown.

!media mesh_hs2.png
  id=mesh_hs2
  caption=Unstructured mesh heat source computed by OpenMC, shown on the bottom of the pincell, with 500000 particles per batch, 50 inactive batches, and 450 active batches
  style=width:55%;margin-left:auto;margin-right:auto

Finally, please note that adding unstructured mesh tallies only affects how the
heat source is measured in OpenMC - the use of unstructured mesh tallies has no
bearing on the temperature and density resolution going *into* OpenMC. For this
example, the temperature will have the same resolution as shown in
[lwr_solid_temp_mc], albeit with slightly different values because the use of
unstructured mesh tallies changes the coupled solution from the case with cell
tallies due to the different resolution of the heat source.

