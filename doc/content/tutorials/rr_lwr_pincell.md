# LWR Pincell with OpenMC's Random Ray Solver

In this tutorial, you will learn:

- How to convert a continuous energy Monte Carlo model to a multi-group [!ac](TRRM) model
- How to couple OpenMC's [!ac](TRRM) solver to MOOSE with a fission heat source and temperature feedback
- The advantages and disadvantages of [!ac](TRRM) method compared to continuous energy Monte Carlo

To access this tutorial,

```
cd cardinal/tutorials/rr_lwr_pincell
```

!alert! note title=Previous Experience
In this tutorial, we assume that the user is familiar with coupling OpenMC to MOOSE in
Cardinal for [!ac](LWR) problems. If you haven't done so already, we recommend that you
complete the [continuous energy LWR pincell tutorial](pincell1.md) before going over
this tutorial.
!alert-end!

## Geometry and Computational Models id=model_1

This model consists of a single UO@2@ pincell from a continuous energy version of the 3D
C5G7 benchmark in [!cite](c5g7). Instead of using the multi-group cross sections from the
C5G7 benchmark specifications, the material properties in [!cite](c5g7_materials) are used
to demonstrate one way to convert from a continuous energy model to a multi-group [!ac](TRRM)
model. The pincel consists of a UO@2@ fuel pellet clad with Zr (with a He gap) surrounded by
borated water. The relevant dimensions of the problem can be found in [table1].

!table id=table1 caption=Geometric specifications for the [!ac](LWR) pincell
| Parameter | Value (cm) |
| :- | :- |
| Fuel pellet outer radius | 0.4095 |
| Fuel clad inner radius | 0.418 |
| Fuel clad outer radius | 0.475 |
| Pin pitch | 1.26 |
| Fuel height | 200 |

A total core power of 3000 MWth is assumed uniformly distributed among 273 rodded fuel assemblies,
each with 264 pins (which do not have a uniform power distribution due to the control rods). The
tallies added by Cardinal will therefore be normalized according to the per-pin power:

\begin{equation}
\label{eq:1}
q_{pin}=\frac{3000\text{\ MWth}}{n_a n_p}
\end{equation}

where $n_a=273$ is the number of fuel assemblies in the core and $n_p=264$ is the number of pins per
assembly. This tutorial considers heat conduction within the fuel and the cladding, but does not model
heat transport in the coolant. Instead, a convective heat flux boundary condition ($h = 1000$ W m^-2^ K)
with the following fluid temperature

\begin{equation}
\label{eq:2}
T_{fluid}(z) = T_{inlet} + (T_{outlet} - T_{inlet})\left(\frac{z + L/2}{L}\right)
\end{equation}

is applied to the outer surface of the cladding. The common parameters for the model can be found in
`common.i`/`common.py`:

!listing /tutorials/rr_lwr_pincell/common.i

### Continuous Energy OpenMC Model id=openmc_ce

OpenMC's Python [!ac](API) is used to generate the [!ac](CSG) model for this [!ac](LWR) pincell. We begin
by defining a helper function, `build_sr_pin`, which we will use to subdivide the [!ac](CSG) region which
represents the UO@2@ pellet portion of the pincell. This serves two purposes: i) to improve the resolution
of the temperature feedback sent to OpenMC, and ii) to apply spatial discretization for [!ac](TRRM). The
function subdivides the radial dimension of a cylindrical region with equal-volume subdivisions, and the
azimuthal dimension is discretized uniformely.

!listing /tutorials/rr_lwr_pincell/make_openmc_model.py
  start=build_sr_pin
  end=main

Afterwards, we define continuous energy material properties for the UO@2@ fuel (`uo2`), Zr cladding (`zr`),
and borated water coolant (`h2o`). Once materials are defined, we create the surfaces and cells necessary
to build the full pincell with [!ac](CSG) in the radial plane. `build_sr_pin` is then used to create a
universe which is populated with the discretized pincell. In this problem the azimuthal dimension is
symmetrical, so we simply subdivide the fuel into 3 radial regions. The radial geometry is then added
to a lattice to extrude it into 100 layers. The lattice (with 100 axial subdivisions) is placed in a cell
which applies vacuum boundary conditions on the top and bottom of the pincell, which reflective boundary
conditions are applied on the remaining sides of the geometry. Finally, we add the materials and geometry
to a model container and set some simulation settings for the continuous energy model. The final radial
discretization of the OpenMC [!ac](CSG) geometry can be found in [rr_openmc_cells].

!listing /tutorials/rr_lwr_pincell/make_openmc_model.py
  start=## Fuel region: UO2 at ~1% enriched.
  end=## Write the continuous-energy model if not running random ray.

!media rr_openmc_cells.png
  id=rr_openmc_cells
  caption=Radial source region discretization used by the [!ac](TRRM) model.
  style=width:60%;margin-left:auto;margin-right:auto;halign:center

### Converting to a Random Ray Model id=openmc_rr

Converting the continuous energy model into a multi-group [!ac](TRRM) model is streamlined by the
autoconversion utilities provided by OpenMC. We create a copy of the continuous energy model with
`rr_model = copy.deepcopy(ce_model)` (to avoid overwriting the continuous energy model) and then call
`rr_model.convert_to_multigroup(...)` followed by `rr_model.convert_to_random_ray(...)`.

!listing /tutorials/rr_lwr_pincell/make_openmc_model.py
  start=rr_model = copy.deepcopy(ce_model)
  end=## Create a mesh to do cell-under-voxel decomposition of the moderator

`rr_model.convert_to_multigroup(...)` runs a series of OpenMC continuous energy calculations to generate
[!ac](MGXS). We choose the `material_wise` option, which runs the provided model (with no modifications)
to generate [!ac](MGXS) for each material. This is the highest fidelity option provided by the
autoconversion function, taking into account both self-shielding and element-shielding effects. Additionally,
`material_wise` is the only option which supports the generation of [!ac](MGXS) for k-eigenvalue problems (the
other options run fixed source calculations). We select the `CASMO-2` group structure for the energy
discretization to reduce the runtime of this tutorial. This is a fairly coarse group structure; we recommend
using more energy groups to minimize bias on power distributions and integral quantities as two groups will
not be sufficient for most problems (without the use of transport equivalences). Finally, we specify
a list of temperatures to use when generating [!ac](MGXS). `convert_to_multigroup` will run a continuous energy
OpenMC simulation for each provided temperature where every material in the model is set to that temperature
(isothermal conditions). This proves to be a severe approximation for multiphysics calculations where large
temperature gradients are experiences between different materials, such as [!ac](LWR) problems (where the
coolant is on the order of ~600 K and the fuel is on the order of ~1400 K). We chose to use it in this tutorial
to simplify the generation of [!ac](MGXS) - care should be exercised if using this option for production
calculations. We recommend reading the
[OpenMC user guide](https://docs.openmc.org/en/stable/usersguide/random_ray.html#generating-multigroup-cross-sections-mgxs)
for generating [!ac](MGXS) to review the pros and cons of the autoconvert function. A final choice that we make
for [!ac](MGXS) generation is the scattering treatment - we select a P@0@ transport correction (`correction="P0"`)
to minimize bias introduced in the scattering source. This is often sufficient for [!ac](LWR) applications.

Converting the model to a random ray model requires no user knowledge compared to the multi-group conversion.
OpenMC will examine the bounding box of our pincell to determine the maximum chord length, and set the inactive
length to that distance. The maximum chord length is often larger then ten mean free paths in reactor applications,
and so ray initialization bias is wiped away with this choice. The active distance is set to five times the maximum
chord length. Unlike the conversion to multi-group where [!ac](MGXS) are generated, the selection of the inactive
and active ray length can be set to different values if they are found to be insufficient. This function also sets
the ray source to the geometric bounding box, and takes a guess as to how many rays will be required. The
number of rays should be set by the user as this guess is almost never sufficient.

After converting to a [!ac](TRRM) model, we set some discretization parameters specific to our problem. This
includes  further source region subdivision of the coolant region using a uniform mesh (known as
[cell-under-voxel source region decomposition](https://docs.openmc.org/en/stable/usersguide/random_ray.html#subdivision-of-source-regions)).
This discretization is easy to apply, however it can yield very small source regions near the intersection of the
boundaries of a mesh voxel and a [!ac](CSG) cell (which is less likely to get hit by a ray). This may result in
stability issues in some cases and worse statistics for tallies. An additional disadvantage of this subdivision
approach is that Cardinal cannot map to these regions as they are created on the fly during the simulation
process; we recommend manual subdivision for most multiphysics simulations unless the regions do not require mapped
tallies or temperature/density feedback. We also set the source region discretization to either: i) flat sources,
or ii) linear sources (depending on command line arguments provide to the Python script). Linear sources will result
in an increase in accuracy for a given geometric discretization at the cost of increasing computational time, and
potentially introducing negative tally values. These negative values are a symptom of a very coarse source region
discretization. Finally, we set `random_ray['diagonal_stabilization_rho'] = 1.0` to ensure our use of a transport
correction doesn't destabilize the combined scattering source / power iteration used by [!ac](TRRM).

!listing /tutorials/rr_lwr_pincell/make_openmc_model.py
  start=## Create a mesh to do cell-under-voxel decomposition of the moderator
  end=if __name__ == "__main__":

The flat source random ray model can be generated with,

```bash
python3 make_openmc_model -r
```

and the linear source model can be generated with,

```bash
python3 make_openmc_model -r --linear
```

### MOOSE Heat Conduction Model id=heat_cond

!include steady_hc.md

Because heat transfer and fluid flow in the borated water is not modeled in this example, heat removal by the fluid
is approximated by setting the outer surface of the cladding to a convection boundary condition,

\begin{equation}
\label{eq:5}
q^{''}=h\left(T-T_{fluid}\right)
\end{equation}

where $h$ and $T_{fluid}$ are described in the model parameters.

!include radiation_gap.md

## Multiphysics Coupling id=couple

### Mesh id=mesh

In this multiphysics model, the heat conduction and neutronics solves use the same mesh. This is generated with a standalone
input file (`solid_mesh.i`):

!listing /tutorials/rr_lwr_pincell/solid_mesh.i

The pincell is created first with a [PolygonConcentricCircleMeshGenerator](PolygonConcentricCircleMeshGenerator.md),
where the radial regions have the same radii as the source region discretization we set up in the OpenMC [!ac](CSG)
model. This ensures that we maintain a one-to-one mapping between subdomains in the MOOSE mesh and the OpenMC geometry
to avoid warping tallies. The mesh is then extruded axially with an [AdvancedExtruderGenerator](AdvancedExtruderGenerator.md).
We label the boundary between the fuel and the gap (`fuel_or`), the boundary between the clad and the gap (`clad_ir`),
and the boundary between the clad and the coolant (`clad_or`). These sidesets are used to apply the gap conductance
model and the convective boundary condition in the heat conduction input file. We then delete the gap and coolant blocks
as they are unused in both the heat conduction application and OpenMC application. Finally, we scale the mesh such that
it is in meters (to match the provided thermal properties). The resulting mesh can be found in [rr_solid_mesh]. Note
that this mesh is coarse for the purpose of reducing the computational burden in this tutorial, and requires additional
refinement to improve temperature predictions.

!media rr_solid_mesh.png
  id=rr_solid_mesh
  caption=Solid mesh for the OpenMC and heat conduction problems. Red and white: UO@2@ fuel. Green: Zr clad.
  style=width:50%;margin-left:auto;margin-right:auto;halign:center

To generate the mesh, run

```
cardinal-opt -i solid_mesh.i --mesh-only
```

### Neutronics Input File id=openmc_input

The neutronics physics is solved over the entire domain using OpenMC; the OpenMC wrapping is described in `openmc.i`.
We begin by loading the mesh that OpenMC will receive temperature from the coupled MOOSE application, and on which
OpenMC will write the fission heat source:

!listing /tutorials/rr_lwr_pincell/openmc.i
  block=Mesh

Next, we define an AuxVariable to use to visualize the temperature Cardinal is setting in OpenMC cells, and write
to it with a [CellTemperatureAux](CellTemperatureAux.md) (which queries the cell temperature in the OpenMC geometry):

!listing /tutorials/rr_lwr_pincell/openmc.i
  start=[AuxVariables]
  end=[Problem]

Afterwards, we replace the default finite element problem with an [OpenMCCellAverageProblem](OpenMCCellAverageProblem.md).
This class is responsible for initializing OpenMC, setting cell temperatures and densities (with equivalent field
variables on the `[Mesh]`), and writing results from OpenMC tallies to the MOOSE mesh. We start by overriding the
number of inactive batches, the total number of batches, and the number of particles per batch. A large number of
inactive batches (in this case, 400) are required when running [!ac](TRRM) as both the scattering and fission sources
must be converged. Fewer rays per batch (1000 in this case) are required as rays do not terminate until they reach
the end of the active length (unlike particles, which are killed on absorption). We then specify `scaling = 100.0`
to convert from mesh units (m) to OpenMC units (cm). We also set the pin power used to normalize tally scores.
Afterwards, we set the cell level of the cell to element mapping to the lowest level in the OpenMC geometry, and
specify that we wish couple OpenMC to the fuel (`uo2` and `uo2_tri` blocks) with temperature feedback. To speed
up the rate of convergence and avoid the nonlinear feedback instabilities inherent to [!ac](LWR) multiphysics
calculations, we use constant relaxation with a relaxation factor of 0.5. The final component of the OpenMC
problem setup is to add a [CellTally](CellTally.md) which accumulates `kappa_fission` over `block = 'uo2 uo2_tri'`.
In addition to the fission heating, we enable outputs for the standard deviation and relative error of the heating
values. Finally, we disable tally global normalization as it is not supported by the [!ac](TRRM) wrapper.

!listing /tutorials/rr_lwr_pincell/openmc.i
  block=Problem

The specification of temperature blocks in the `[Problem]` will add a variable named `temp` to the mesh, which
the [OpenMCCellAverageProblem](OpenMCCellAverageProblem.md) will query to set cell temperatures. To avoid
an initial temperature of zero being sent to OpenMC, we add a [ConstantIC](ConstantIC.md) which sets this field
variable to the average fluid temperature:

!listing /tutorials/rr_lwr_pincell/openmc.i
  block=ICs

The next set of blocks we add setup the coupling scheme. We add a [TransientMultiApp](TransientMultiApp.md) for
the heat conduction solve, which receives fission heating from `heat_source_to_solid` (a
[MultiAppGeneralFieldShapeEvaluationTransfer](MultiAppGeneralFieldShapeEvaluationTransfer.md)) and sends the fuel
temperature to `temp` via `temp_to_openmc` (another
[MultiAppGeneralFieldShapeEvaluationTransfer](MultiAppGeneralFieldShapeEvaluationTransfer.md)). The heat conduction
application is executed after running OpenMC (on `TIMESTEP_END`), resulting in the following coupling scheme:

1. Run OpenMC;
2. Send fission power to heat conduction application (`solid`);
3. Compute fuel temperatures;
4. Send fuel temperatures back to OpenMC;
5. Repeat until fields are converged.

!listing /tutorials/rr_lwr_pincell/openmc.i
  start=[MultiApps]
  end=[Postprocessors]

The final portion of the input file deals with execution, post-processing, and outputs. We use a
[Transient](Transient.md) executioner in this problem to control the number of iterations between
the OpenMC main application and the heat conduction sub-application. In this problem, 5 iterations
are sufficient to converge tally statistics and $k_{eff}$ due to the low variance of [!ac](TRRM).
It should be noted that the problem OpenMC is solving is still a quasi-static k-eigenvalue
calculation, and the heat conduction simulation is steady-state event with the use of a
[Transient](Transient.md) executioner. We add a series of post-processors to monitor the relative
errors of tally variables and report the k-eigenvalue (and its associated standard deviation).
Finally, we select Exodus and CSV output in the `[Outputs]` block.

!listing /tutorials/rr_lwr_pincell/openmc.i
  start=[Postprocessors]

### Solid Input File id=solid_input

The heat conduction problem is setup in the `solid.i` input file, and is very similar to the
[continuous energy LWR pincell tutorial](pincell1.md). It begins by loading   the previously
generated mesh.

!listing /tutorials/rr_lwr_pincell/solid.i
  block=Mesh

Afterwards, nonlinear and auxiliary variables are added for the temperature and heat source
(respectively). This is followed by the addition of kernels to assemble the heat conduction
equation over the fuel and cladding (where the fission heat source `CoupledForce` is block
restricted to the fuel).

!listing /tutorials/rr_lwr_pincell/solid.i
  start=[Variables]
  end=[Functions]

We then add a [ParsedFunction](MooseParsedFunction.md) to represent [eq:2], and a
[ConvectiveFluxFunction](ConvectiveFluxFunction.md) to apply the convective boundary condition
to the outer radius of the clad.

!listing /tutorials/rr_lwr_pincell/solid.i
  start=[Functions]
  end=[Materials]

A series of [GenericConstantMaterial](GenericConstantMaterial.md) are then added to set the
thermal conductivity of the fuel and cladding, which is followed by the addition of a
[GapHeatTransfer](GapHeatTransfer.md) to set up the gap conductance model.

!listing /tutorials/rr_lwr_pincell/solid.i
  start=[Materials]
  end=[Executioner]

Finally, the `[Executioner]` is set to [Transient](Transient.md). As no timestepping parameters
are provided, the heat conduction simulation will execute in lock-step with the OpenMC simulation.
We add several post-processors to monitor the maximum fueld and cladding temperature, and select
Exodus and CSV output in the `[Outputs]` block.

!listing /tutorials/rr_lwr_pincell/solid.i
  start=[Executioner]

## Execution and Postprocessing id=results

To run the coupled calculation with [!ac](TRRM) and flat sources,

```bash
python3 make_openmc_model -r
cardinal-opt -i solid_mesh.i --mesh-only
mpiexec -np 2 cardinal-opt -i openmc.i --n-threads=2
```

To run with [!ac](TRRM)  and linear sources,

```bash
python3 make_openmc_model -r --linear
cardinal-opt -i solid_mesh.i --mesh-only
mpiexec -np 2 cardinal-opt -i openmc.i --n-threads=2
```

and to run the continuous energy reference,

```bash
python3 make_openmc_model
cardinal-opt -i solid_mesh.i --mesh-only
mpiexec -np 2 cardinal-opt -i openmc.i --n-threads=2
```

All of the above will use 2 MPI ranks and 2 OpenMP threads per rank. To run the simulation
faster, you can increase the number of ranks/threads or decrease the number of particles. Bear
in mind that decreasing the number of particles may result in a failure to converge when running
[!ac](TRRM) as fewer source regions will be hit on each power iteration.
