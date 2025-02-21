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
The full assembly case requires [!ac](HPC) due to the number of elements in the tally mesh, both before and after the application
of [!ac](AMR). You can run the OpenMC problem with a reduced number of particles, however the mesh may not refine due to
the large tally relative errors caused by poor stastistics. We include a section on running [!ac](AMR) on a single pin of
the assembly mesh if you don't have access to [!ac](HPC) resources - this example can be found in [#pincell].
!alert-end!

## Geometry and Computational Models id=model

This model consists of a single UO$_2$ assembly from the 3D C5G7 extension case in [!cite](c5g7), where control
rods are fully inserted in the assembly to induce strong axial and radial gradients. Instead of using the multi-group
cross sections from the C5G7 benchmark specifications, the material properties in [!cite](c5g7_materials) are used.
At a high level the geometry consists of the following lattice elements (in a 17x17 grid):

- 264 fuel pins composed of UO$_2$ pellets clad in zirconium with a helium gap;
- 24 control rods composed of B$_4$C pellets clad in aluminum (with no gap) which occupy the assembly guide tubes;
- A single fission chamber in the central guide tube composed of borated water with a trace amount of U-235 clad in aluminum (with no gap).

The remainder of the assembly which is not filled with these pincells is composed of borated water. Above the top of the fuel there is a
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

### OpenMC Model id=openmc

OpenMC's Python [!ac](API) is used to generate the [!ac](CSG) model for this [!ac](LWR) assembly. We begin by defining the
geometric specification of the assembly. This is followed by the creation of materials for each of the regions
described in the previous section, where the helium gap is assumed to be well represented by a void region. We then define the geometry
for each pincell (fuel, control rod in guide tube, fission chamber, reflector water) which are used to define the 17x17 lattice
for both the active fuel region and the reflector. The sides and bottom of the assembly use reflective boundary conditions while
the top of the assembly uses a vacuum boundary condition (in other words, we are modeling only half the axial extent of the bundle). The OpenMC model can be found in [assembly_amr_openmc]. The Python script used
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

### Mesh Generation with the Reactor Module id=mesh

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
and so they will not receive any tallies (in other words, we are simply reducing some memory needed to keep unused parts of the mesh, though you could certainly keep those mesh blocks if you desired). We finish the mesh generation process by translating the assembly mesh such that its center is located at
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
  caption=Initial tally mesh colored by block ID, shown in an isometric view and sliced on the $x-y$ plane
  style=width:90%;margin-left:auto;margin-right:auto

### Neutronics Input File id=neutronics

The neutronics calculation is performed over the entire assembly by OpenMC, and the wrapping of the OpenMC results in MOOSE is performed in
`openmc.i`. We begin by defining the mesh which will be used by OpenMC to tally the volumetric fission power, which we generated in the
previous step:

!listing /tutorials/lwr_amr/openmc.i
  block=Mesh

Next, the [Problem](Problem/index.md) and [Tallies](AddTallyAction.md) blocks describe all of the syntax necessary to replace the normal
MOOSE finite element calculation with an OpenMC neutronics solve. This is done with an [OpenMCCellAverageProblem](OpenMCCellAverageProblem.md).

!listing /tutorials/lwr_amr/openmc.i
  block=Problem

We specify the number of particles to use alongside the assembly power to normalize tally results. We set `normalize_by_global_tally = false`
as the unstructured mesh tally does not perfectly represent the [!ac](CSG) geometry, and so some scores will be missed compared to a global tally.
`assume_separate_tallies` and `skip_statepoint` are set to `true` as performance optimizations. A [MeshTally](MeshTally.md) is added in the
`[Tallies]` block which will automatically score on `mesh_in.e`. This tally scores `kappa_fission` to a MOOSE variable named `heat_source`
while also calculating the standard deviation of the tally field (stored in `heat_source_std_dev`). We specify that we only wish to tally on
the fueled regions (`uo2_center` and `uo2`) as the remainder of the assembly will accumulate zero (or near-zero in the case of the fission chamber)
tally scores for fission heating.

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

### Execution and Postprocessing id=exec

To run the wrapped neutronics calculation,

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
if this follows the standard cosine shape, and the lack of radial refinement fails to capture gradients within each pincell. This
motivates the use of [!ac](AMR) to automatically refine the tally mesh. 

To add [!ac](AMR), we add two sub-blocks. The first is the
[Indicators](https://mooseframework.inl.gov/syntax/Adaptivity/Indicators/index.html) block, members of which are responsible for
computing estimates of the spatial solution error for each element in the mesh. The second is the
[Markers](https://mooseframework.inl.gov/syntax/Adaptivity/Markers/index.html) block, members of which take error estimates from
indicators and use them to determine if an element should be isotropically refined or coarsened.

!media assembly_amr_init_res.png
  id=assembly_amr_init_res
  caption=Fission power computed with the unstructured mesh tally, shown with an isometric view and sliced on the $x-y$ plane
  style=width:90%;margin-left:auto;margin-right:auto

## Adding Adaptive Mesh Refinement

Next, we will adaptively refine the mesh using the adaptivity system included in MOOSE. The Cardinal input file for adaptivity is
largely the same as the input file for the initial mesh tally - it can be found in `openmc_amr.i`. The main difference between
`openmc.i` and `openmc_amr.i` is the addition of the [Adaptivity](https://mooseframework.inl.gov/syntax/Adaptivity/index.html)
block (shown below), which is responsible for determining the refinement / coarsening behaviour of the mesh. The adaptivity block
consists of two sub-blocks: [Indicators](https://mooseframework.inl.gov/syntax/Adaptivity/Indicators/index.html) which compute estimates
of spatial discretization error, and [Markers](https://mooseframework.inl.gov/syntax/Adaptivity/Markers/index.html) which use
these error estimates to mark elements for refinement or coarsening. Two steps of mesh adaptivity are selected with `steps = 2`,
and `error_combo` is selected to be the marker to use when modifying the mesh.

!listing /tutorials/lwr_amr/openmc_amr.i
  block=Adaptivity

In this adaptivity block we define a single indicator which
assumes that the spatial error in an element is proportional to the mean optical depth within the element (`optical_depth`). This
indicator requires a tally score to use for computing the energy-integrated reaction rate in an element - we select the `fission`
reaction rate as we aim to optimize the spatial distribution of fission heating. It also requires an estimate of the mean chord
length in the element (`h_type`) which we set to the maximum vertex separation in an element due to the coarse axial discretization
in our initial mesh.

We then add three markers, the first of which is a
[ErrorFractionMarker](https://mooseframework.inl.gov/source/markers/ErrorFractionMarker.html) (`depth_frac`) which takes `optical_depth`
as an input. This marker sorts all elements into descending order based on the indicator value, then iterates over the list
(starting with the largest error estimate). Elements whose optical depth sum to a refinement fraction multiplied by the total
error estimate are refined. The list is iterated in reverse to mark elements for coarsening. We set the refinement fraction
`refine = 0.3`, while setting the coarsening threshold `coarsen = 0.0`. The next marker we add is a
[ValueThresholdMarker](https://mooseframework.inl.gov/source/markers/ValueThresholdMarker.html) (`rel_error`), which marks elements for
refinement if their value is above `refine` and marks elements to be coarsened if their value is above `coarsen`. We pass it
the variable containing the relative error of the fission heating (`variable = heat_source_rel_error`) and set `invert = true`.
This has the effect of marking an element for coarsening if the fission heating relative error is above `1e-1`, while allowing
refinement of the relative error of the fission heating relative error is below `5e-2`. We set `third_state = DO_NOTHING` to
indicate that elements in this region should not be refined or coarsened. The final marker we add is a
[BooleanComboMarker](BooleanComboMarker.md) (`error_combo`) which combines the previous two markers based on a flag in `boolean_operator`.
`refine_markers = 'rel_error depth_frac'` with the `and` boolean flag results in refinement only when the element is marked based
on it's fraction of the optical depth and has a sufficiently low statistical relative error. `coarsen_markers = 'rel_error'`
results in coarsening only if the element has a sufficiently hight statistical relative error.

The remaining modifications that need to be made are done to support `[Adaptivity]`. We modify `heat_source` to also score `flux` and `fission`
to allow for the calculation of the optical depth. This also necessitates the specification of `source_rate_normalization = 'kappa_fission'`
as two non-heating scores have been added. `unrelaxed_tally_rel_error` is added to the `output` parameter to enable the `rel_error` marker.
The modifications to the `[Problem]` and `[Tallies]` block can be found below.

!listing /tutorials/lwr_amr/openmc_amr.i
  block=Problem

The final change we make is to add an extra postprocessor named `num_elem` to output the number of elements. This postprocessor filters for
the number of *active* elements, which are the elements currently being tallied on. This is distinct from the *total* number of elements as
MOOSE/libMesh maintains the entire refinement hierarchy, and so elements which aren't participating in the solve at the current refinement
cycle still exist in memory (and therefore are included in the total element count).

!listing /tutorials/lwr_amr/openmc_amr.i
  block=Postprocessors

### Execution and Postprocessing

To run the neutronics calculation with [!ac](AMR),

```bash
mpiexec -np 4 cardinal-opt -i openmc_amr.i --n-threads=32
```

First, lets examine how the reported relative errors and the number of elements in the problem changes as the mesh is
adaptively refined / coarsened:

```
Postprocessor Values:
+----------------+----------------+----------------+----------------+----------------+
| time           | avg_rel_err    | max_rel_err    | min_rel_err    | num_elem       |
+----------------+----------------+----------------+----------------+----------------+
|   1.000000e+00 |   1.721342e-02 |   3.750385e-02 |   8.727138e-03 |   4.624000e+04 |
|   2.000000e+00 |   4.803123e-02 |   1.500115e-01 |   2.182817e-02 |   1.940800e+05 |
|   3.000000e+00 |   6.194000e-02 |   1.979877e-01 |   2.188693e-02 |   2.466990e+05 |
+----------------+----------------+----------------+----------------+----------------+
```

We can see that `avg_rel_err` starts at ~1% on the first iteration and jumps to ~4% on the second iteration, which is corroborated by
`num_elem` quadrupling on this step (from 46240 to 194080) indicating that a large amount of refinement is taking place.
This behavior slows down on the third step as the number of elements remaining that meet both the spatial refinement criteria (optical depth)
and the statistical error requirement (relative error less than 5%) are limited, and so the average relative error only increases to ~6% while another
52619 elements are added. Looking at `max_rel_err` we can also see that some elements meet the statistical error criteria before refinement,
but after refinement their relative error overshoots the maximum error threshold and so the maximum relative error in the problem jumps to
~20%. We can see this behavior in [assembly_amr_res] where the heat source is initially refined semi-uniformly over the domain. Then, the
elements near the core centerline and the edges of the assembly are marked for refinement due to an increase in the estimated optical depth
from higher fission rates. After the third iteration the edge of the assembly nearest to the vacuum boundary and the corners of the assembly
are marked for coarsening due to the jump in relative error. Running the simulation with additional particles per batch or more active batches
would further decrease the per-element relative error and allow for added refinement steps. The jumps in relative error above the upper limit of 10%
can also be mitigated by decreasing the refinement threshold in `rel_error` from 5% to 1%.

!media assembly_amr_res.gif
  id=assembly_amr_res
  caption=Elements marked for refinement (left), fission power (middle), and elements marked for coarsening (right) for the [!ac](LWR) assembly
  over multiple adaptivity cycles.
  style=width:100%;margin-left:auto;margin-right:auto

## Single Pincell id=pincell

In this section, we isolate a single pin from the corner of the assembly mesh to tally and run adaptivity on. The mesh input file
(`mesh_pin.i`) can be found below. We first generate a UO$_2$ pin, and translate it to ensure it lines up with a fuel pin (as the
central pin in the assembly is a fission chamber). This pin is then extruded to the full length of the fueled region and the gap
block is deleted (as it will never be tallied on).

!listing /tutorials/lwr_amr/mesh_pin.i

The mesh can be generated by running:

```bash
cardinal-opt -i mesh_pin.i --mesh-only
```

The first modification made to `openmc_amr.i` is to change the mesh block such that it tallies on the single pin mesh:

!listing /tutorials/lwr_amr/openmc_amr_single.i
  block=Mesh

The second modification is to set `particles = 1000` to ensure this problem is runnable with modest computational resources:

!listing /tutorials/lwr_amr/openmc_amr_single.i
  block=Problem

### Execution and Postprocessing

To run the single pin problem,

```bash
mpiexec -np 2 cardinal-opt -i openmc_amr_single.i --n-threads=2
```

First, lets examine how the reported relative errors and the number of elements in the problem changes as the mesh is
adaptively refined / coarsened:

```
Postprocessor Values:
+----------------+----------------+----------------+----------------+----------------+
| time           | avg_rel_err    | max_rel_err    | min_rel_err    | num_elem       |
+----------------+----------------+----------------+----------------+----------------+
|   1.000000e+00 |   5.435691e-02 |   1.121199e-01 |   2.985789e-02 |   1.600000e+02 |
|   2.000000e+00 |   9.696449e-02 |   1.707078e-01 |   4.991084e-02 |   4.120000e+02 |
|   3.000000e+00 |   9.169795e-02 |   1.555218e-01 |   4.561517e-02 |   3.770000e+02 |
+----------------+----------------+----------------+----------------+----------------+
```

The behavior of the relative error for the single pincell is similar to the full assembly case, though the reduced number
of particles per batch decreases the amount of refinement that the model can support. `avg_rel_err` starts at ~5% on the first
iteration, and then jumps to ~9% due to the addition of more elements on the second iteration. The selected adaptivity scheme
then coarsens those elements which results in a decrease in `max_rel_err` from ~17% on iteration two to ~15% on iteration three
- the maximum doesn't decrease a substantial amount as certain elements have a relative error less than 5% and refine to a relative
error larger than 10%. We can see this behaviourin [pincell_amr_res], where the adaptivity algorithm initially over-refines and
then coarsens to ensure the relative error of the tally bins is between 5% and 10%

!media pincell_amr_res.gif
  id=pincell_amr_res
  caption=Elements marked for refinement (left), fission power (middle), and elements marked for coarsening (right) for a single
  pincell over multiple adaptivity cycles.
  style=width:100%;margin-left:auto;margin-right:auto
