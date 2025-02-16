# Adaptive Mesh Refinement on Mesh Tallies

In this tutorial, you will learn how to:

- Couple OpenMC to MOOSE (via a heat source) for a single assembly in the C5G7 [!ac](LWR) reactor physics benchmark
- Generate assembly meshes using the MOOSE [Reactor module](https://mooseframework.inl.gov/modules/reactor/index.html)
- Enable [!ac](AMR) on an unstructured mesh tally to automatically refine the heat source

To access this tutorial,

```
cd cardinal/tutorials/lwr_amr
```

!alert! note title=Computing Needs
This tutorial requires [!ac](HPC) due to the number of elements in the tally mesh, both before and after the application
of [!ac](AMR). You can run the OpenMC problem with a reduced number of particles, however the mesh may not refine due to
the large tally relative errors caused by poor stastistics.
!alert-end!

## Geometry and Computational Models

This model consists of a single UO$_2$ assembly from the 3D C5G7 extension case in [!cite](c5g7), where control
rods are fully inserted in the assembly to induce strong axial and radial gradients. Instead of using the multi-group
cross sections from the C5G7 benchmark specifications, the material properties in [!cite](c5g7_materials) are used.
At a high level the geometry consists of the following lattice elements (in a 17x17 grid):

- 264 fuel pins composed of UO$_2$ pellets clad in zirconium with a helium gap;
- 24 control rods composed of BC$_4$ pellets clad in aluminum (with no gap) which occupy the assembly guide tubes;
- A single fission chamber in the center guide tube composed of borated water with a trace amount of U-235 clad in aluminum (with no gap).

The remainder of the assembly which is not filled with these pincells is composed of water. Above the top of the fuel there is a
reflector region which is penetrated by the inserted control rods. The relevant dimensions can be found in [table1].

!table id=table1 caption=Geometric specifications for a rodded [!ac](LWR) assembly
| Parameter | Value (cm) |
| :- | :- |
| Fuel pellet outer radius | 0.4095 |
| Fuel clad inner radius | 0.418 |
| Fuel clad outer radius | 0.475 |
| Control rod outer radius | 0.3400 |
| Fission chamber outer radius | 0.3400 |
| Guide tube clad outer radius | 0.54 |
| Pin pitch | 1.26 |
| Fuel height | 192.78 |
| Reflector thickness | 21.42 |

A total core power of 3000 MWth is assumed uniformly distributed among 273 rodded fuel assemblies, each with 264 pins (which do not have a
uniform power distribution due to the control rods). The unstructured mesh tallies added by Cardinal will therefore be normalized
according to the per-assembly power:

\begin{equation}
\label{eq:1}
q_{as}=\frac{3000\text{\ MWth}}{n_a}
\end{equation}

where $n_a=273$ is the number of fuel assemblies in the core.

### OpenMC Model

OpenMC's Python [!ac](API) is used to generate the [!ac](CSG) model for this [!ac](LWR) assembly. We begin by defining the
geometric specification of the assembly. This is followed by the creation of materials for each of the regions
described in the previous section, where the helium gap is assumed to be well represented by a void region. We then define the geometry
for each pincell (fuel, control rod in guide tube, fission chamber, reflector water) which are used to define the 17x17 lattice
for both the active fuel region and the reflector. The sides and bottom of the assembly use reflective boundary conditions while
the top of the assembly uses a vacuum boundary condition. The OpenMC model can be found in [assembly_amr_openmc]. The Python script used
to generate the `model.xml` file can be found below.

!listing /tutorials/lwr_amr/make_openmc_model.py

!media assembly_amr_openmc.png
  id=assembly_amr_openmc
  caption=OpenMC geometry colored by material ID shown on the $x$-$y$ and $x$-$z$ planes
  style=width:90%;margin-left:auto;margin-right:auto

To generate the XML files needed to run OpenMC, you can run the following:

```bash
python make_openmc_model.py
```

Or you can use the `model.xml` file that is included in the `tutorials/lwr_amr` directory.

### Mesh Generation with the Reactor Module

The mesh used to tally the fission heating in this problem will be generated using the MOOSE
[Reactor module](https://mooseframework.inl.gov/modules/reactor/index.html), which provides a suite of mesh generators suited for
common fission reactor geometries. We begin by defining the geometric properties of the assembly (as described in [table1]) followed
by some discretization parameters. `NUM_SECTORS` is the number of azimuthal discretization regions per for a quadrant of the pincell,
`FUEL_RADIAL_DIVISIONS` is the number of radial regions we use for subdividing the fuel, `BACKGROUND_DIVISIONS` is the number of radial
subdivisions for the water bounding box, and `AXIAL_DIVISIONS` is the number of subdivisions we use when extruding the geometry into 3D.

!listing /tutorials/lwr_amr/mesh.i
  end=[Mesh]

After defining the geometric properties and discretization parameters we define the four different pincells in the `[Mesh]` block using the
[PolygonConcentricCircleMeshGenerator](https://mooseframework.inl.gov/source/meshgenerators/PolygonConcentricCircleMeshGenerator.html) -
the workhorse of the [Reactor module](https://mooseframework.inl.gov/modules/reactor/index.html). We set `preserve_volumes = true` in an
attempt to preserve the volume of each region, which ensures that our tallied volumetric fission power is consistent with the OpenMC geometry.

!listing /tutorials/lwr_amr/mesh.i
  start=[UO2_Pin]
  end=[UO2_Assembly]

After defining each pincell, these distinct regions are combined into a 2D assembly mesh using a
[PatternedCartesianMeshGenerator](https://mooseframework.inl.gov/source/meshgenerators/PatternedCartesianMeshGenerator.html).

!listing /tutorials/lwr_amr/mesh.i
  start=[UO2_Assembly]
  end=[Delete_Blocks]

The fuel-cladding gap blocks are deleted after the assembly mesh is generated - this is due to these regions being modelled as voids in OpenMC
and so they will not receive any tallies. We finish the mesh generation process by translating the assembly mesh such that it's center is located at
the origin, and extrude the geometry from the fuel centerline to the top of the active fuel region.

!listing /tutorials/lwr_amr/mesh.i
  start=[Delete_Blocks]

We generate the mesh by running the command below, which creates the `mesh_in.e` file that is used in the Cardinal input file for volumetric
tallies. The final mesh can be found in [assembly_amr_init_mesh], which is a fairly coarse mesh in the axial direction. We will rely on
[!ac](AMR) to refine both the radial and axial power distribution.

```bash
cardinal-opt -i mesh.i --mesh-only
```

!media assembly_amr_init_mesh.png
  id=assembly_amr_init_mesh
  caption=Initial tally mesh colored by block ID shown in an isometric view and sliced on the x-y plane
  style=width:90%;margin-left:auto;margin-right:auto

### Neutronics Input File

The neutronics calculation is performed over the entire assembly by OpenMC, and the wrapping of the OpenMC results in MOOSE is performed in
`openmc.i`. We begin by defining the mesh which will be used by OpenMC to tally the volumetric fission power, which we generated in the
previous step:

!listing /tutorials/lwr_amr/openmc.i
  block=Mesh

Next, the [Problem](Problem/index.md) and [Tallies](AddTallyAction.md) blocks describe all of the syntax necessary to replace the normal
MOOSE finite element calculation with an OpenMC neutronics solve. This is done with an [OpenMCCellAverageProblem](OpenMCCellAverageProblem.md).

!listing /tutorials/lwr_amr/openmc.i
  block=Problem

We specify that we wish to use `10000` particles per batch with `50` inactive batches (to converge the fission source) and `1000` total batches
(`950` batches accumulating statistics) - this is due to the spatially small tally bins. We then specify the total power of the assembly
in `power` to normalize tally results. We set `normalize_by_global_tally = false` because the unstructured mesh tally does not perfectly
represent the [!ac](CSG) geometry, and so some scores will be missed. `assume_separate_tallies` and `skip_statepoint` are set to `true`
as performance optimizations. A [MeshTally](MeshTally.md) is added in the `[Tallies]` block which will automatically score on `mesh_in.e`.
This tally scores `kappa_fission` to a MOOSE variable named `heat_source` while also calculating the standard deviation of the tally field
(stored in `heat_source_std_dev`). We specify that we only wish to tally on the fueled regions (`uo2_center` and `uo2`) as the remainder
of the assembly will accumulate zero (or near-zero in the case of the fission chamber) tally scores for fission heating.

A steady-state executioner is selected as OpenMC will run a single criticality calculation. We then select exodus and [!ac](CSV) output (for our
postprocessors) which will occur on the end of the simulation.

!listing /tutorials/lwr_amr/openmc.i
  start=[Executioner]
  end=[Postprocessors]

Finally, we add three [TallyRelativeError](TallyRelativeError.md) postprocessors to evaluate the minimum, maximum, and average tally relative
error of the `kappa_fission` score. These are useful for determining if the number of batches need to be increased when increasing the mesh
resolution using [!ac](AMR).

!listing /tutorials/lwr_amr/openmc.i
  block=Postprocessors

## Execution and Postprocessing

To run the wrapped neutronic calculation,

```bash
mpiexec -np 4 cardinal-opt -i openmc.i --n-threads=32
```

This will run OpenMC with 4 MPI ranks with 32 OpenMP threads per rank. To run the simulation faster, you can increase the parallel
processes/threads, or simply decrease the number of particles used in OpenMC. When the simulation has completed, you will have created
a number of different output files:

- `openmc_out.e`: an Exodus mesh with the tally results;
- `openmc_out.csv`: a [!ac](CSV) file with the results reported by the [TallyRelativeError](TallyRelativeError.md) postprocessors.

[assembly_amr_init_res] shows the resulting fission power computed by OpenMC on the mesh tally added by Cardinal. We can see
that the radial power distribution peaks near the corners of the assembly (due to the reflective boundary conditions), and is
depressed near the guide tubes due to the insertion of the control rods. The center of the assembly sees some additional power
peaking due to the fission chamber. The power distribution within the assembly begins to decrease along the z-axis as one moves
from the assembly centerline to the vacuum boundary condition. The coarse axial discretization makes it difficult to determine
if this follows the standard cosine shape and the lack of radial refinement fails to capture gradients within each pincell. This
motivates the use of [!ac](AMR) to automatically refine the tally mesh near these regions.

!media assembly_amr_init_res.png
  id=assembly_amr_init_res
  caption=Fission power computed with the unstructured mesh tally, shown with an isometric view and sliced on the x-y plane
  style=width:90%;margin-left:auto;margin-right:auto

## Adding Adaptive Mesh Refinement
