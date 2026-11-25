# LWR Pincell with OpenMC's Random Ray Solver

In this tutorial, you will learn:

- How to convert a continuous energy Monte Carlo model to a multi-group [!ac](TRRM) model
- How to couple OpenMC's [!ac](TRRM) solver to MOOSE with a fission heat source, density feedback, and temperature feedback
- The advantages and disadvantages of [!ac](TRRM) compared to continuous energy Monte Carlo

To access this tutorial,

```
cd cardinal/tutorials/rr_lwr_pincell
```

## Geometry and Computational Models id=model_1

This model consists of a single UO@2@ pincell from a continuous energy version of the 3D
C5G7 benchmark in [!cite](c5g7). Instead of using the multi-group cross sections from the
C5G7 benchmark specifications, the material properties in [!cite](c5g7_materials) are used
to demonstrate conversion from a continuous energy model to a multi-group [!ac](TRRM)
model. The pincell consists of a UO@2@ fuel pellet clad with Zr. The pin is surrounded by
borated water, and there is a He gap between the fuel and cladding. The relevant dimensions
of the problem can be found in [table1].

!table id=table1 caption=Geometric specifications for the [!ac](LWR) pincell
| Parameter | Value (cm) |
| :- | :- |
| Fuel pellet outer radius | 0.4095 |
| Fuel clad inner radius | 0.418 |
| Fuel clad outer radius | 0.475 |
| Pin pitch | 1.26 |
| Fuel height | 200 |

A total core power of 3411 MWth is assumed to be uniformly distributed among 273 rodded fuel assemblies,
each with 264 pins (which do not have a uniform power distribution due to the control rods and guide
tubes). The tallies added by Cardinal will therefore be normalized according to the per-pin power:

\begin{equation}
\label{eq:1}
q_{pin}=\frac{3411\text{\ MWth}}{n_a n_p}
\end{equation}

where $n_a=273$ is the number of fuel assemblies in the core and $n_p=264$ is the number of pins per
assembly. The coolant around the pincell is assumed to be flowing with a mass flowrate of 0.875
kg/s, an inlet temperature of 573 K, and an outlet pressure of 15.51 MPa. The common parameters for
the model can be found in `common.i`:

!listing /tutorials/rr_lwr_pincell/common.i

### Continuous Energy OpenMC Model id=openmc_ce

OpenMC's Python [!ac](API) is used to generate the [!ac](CSG) model for this [!ac](LWR) pincell. We begin
by defining a helper function, `build_sr_pin`, which we will use to subdivide the [!ac](CSG) region representing
the UO@2@ pellet portion of the pincell. This serves two purposes: i) to improve the resolution
of the temperature feedback sent to OpenMC, and ii) to apply spatial discretization for [!ac](TRRM). The
function subdivides the radial dimension of a cylindrical region with equal-volume subdivisions, and the
azimuthal dimension is discretized uniformly.

!listing /tutorials/rr_lwr_pincell/make_openmc_model.py
  start=build_sr_pin
  end=main

Afterwards, we define continuous energy material properties for the UO@2@ fuel (`uo2`), Zr cladding (`zr`),
and borated water coolant (`h2o`). Once materials are defined, we create the surfaces and cells necessary
to build the full pincell with [!ac](CSG) in the radial plane. `build_sr_pin` is then used to create a
universe which is populated with the discretized pincell. In this problem the azimuthal dimension of the
fuel pin itself is symmetric, so we simply subdivide the fuel into 3 radial regions. The radial geometry is then added
to a lattice to extrude it into 100 layers axially. The lattice is placed in a cell
which applies vacuum boundary conditions on the top and bottom of the pincell, while reflective boundary
conditions are applied on the remaining sides of the geometry. Finally, we add the geometry
to a model container and set some simulation settings for the continuous energy model. The radial
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
using more energy groups to minimize bias in power distributions and integral quantities as two groups will
not be sufficient for most problems (without the use of transport equivalences). Finally, we specify
a list of temperatures to use when generating [!ac](MGXS). `convert_to_multigroup` will run a continuous energy
OpenMC simulation for each provided temperature where every material in the model is set to that temperature
(isothermal conditions). This proves to be a severe approximation for multiphysics calculations where large
temperature differences exist between different materials, such as [!ac](LWR) problems (where the
coolant has a temperature of ~600 K and the fuel has a temperature of ~1400 K). We chose to use isothermal
conditions in this tutorial to simplify the generation of [!ac](MGXS) - care should be exercised if using this
option for production calculations. We recommend reading the
[OpenMC user guide](https://docs.openmc.org/en/stable/usersguide/random_ray.html#generating-multigroup-cross-sections-mgxs)
for generating [!ac](MGXS) to review the pros and cons of the autoconvert function and to learn how you can generate the
[!ac](MGXS) without relying on the autoconvert function. A final choice that we make
for [!ac](MGXS) generation is the scattering treatment - we select a P@0@ transport correction (`correction="P0"`)
to minimize bias introduced in the scattering source. This is often sufficient for [!ac](LWR) applications.

Converting to a random ray model requires minimal user knowledge. OpenMC will examine the bounding box of our pincell to
determine the maximum chord length, and set the inactive length to that distance. The maximum chord length is often
larger than ten mean free paths in reactor applications, and so
[ray initialization bias](https://docs.openmc.org/en/stable/methods/random_ray.html#ray-starting-conditions-and-inactive-length)
is wiped away with this choice. The active distance is set to five times the maximum chord length. The inactive and active
ray lengths can be set to different values if they are found to be insufficient. This function also sets the ray source to
the geometric bounding box, and takes a guess as to how many rays will be required. The number of rays should
be set by the user as the default guess is almost never sufficient.

After converting to a [!ac](TRRM) model, we set some discretization parameters specific to our problem. This
includes further source region subdivision of the coolant region using a uniform mesh (known as
[cell-under-voxel source region decomposition](https://docs.openmc.org/en/stable/usersguide/random_ray.html#subdivision-of-source-regions)).
This discretization is easy to apply, however it can yield very small source regions near the intersection of the
boundaries of a mesh voxel and a [!ac](CSG) cell (which are less likely to get hit by a ray). This may result in
stability issues in some cases and worse statistics for tallies. An additional disadvantage of this subdivision
approach is that Cardinal cannot map to these regions as they are created on the fly during the simulation
process; we recommend manual subdivision for most multiphysics simulations unless the regions do not require mapped
tallies or temperature/density feedback. We also set the source region discretization to either: i) flat sources,
or ii) linear sources (depending on command line arguments provided to the Python script). Linear sources will result
in an increase in accuracy for a given geometric discretization at the cost of increased computational time, and
potentially introducing negative source values (which OpenMC will set to zero). These negative values are a symptom
of a very coarse source region discretization. Finally, we set `random_ray['diagonal_stabilization_rho'] = 1.0`
to ensure our use of a transport correction doesn't destabilize the combined scattering source/power iteration
used by [!ac](TRRM).

!listing /tutorials/rr_lwr_pincell/make_openmc_model.py
  start=## Create a mesh to do cell-under-voxel decomposition of the moderator
  end=if __name__ == "__main__":

The flat source random ray model can be generated with,

```bash
python make_openmc_model.py -r
```

and the linear source model can be generated with,

```bash
python make_openmc_model.py -r --linear
```

### MOOSE Heat Conduction Model id=heat_cond

!include steady_hc.md

Heat removal by the fluid is applied through the use of a matching value boundary condition, where
the surface temperature of the cladding is set equal to the pin surface temperature computed
by the MOOSE subchannel module. The subchannel model uses a corrected version of the
[Dittus-Boelter correlation](SCMHTCDittusBoelter.md) to compute the fluid Nusselt number
(for the convective heat transer coefficient), and subsequently the cladding surface temperature.

!include radiation_gap.md

### Subchannel Flow Model id=subchannel

The fluid flow around the pincell is computed with the MOOSE
[subchannel module](https://mooseframework.inl.gov/modules/subchannel/index.html), which discretizes
the fluid domain into a series of pseudo-3D subchannels which exchange mass, momentum, and energy
on the radial and axial planes. As the purpose of this tutorial is to demonstrate how one can couple OpenMC's
random ray solver to other physics, we refer readers to the subchannel
[theorey manual](https://mooseframework.inl.gov/modules/subchannel/general/subchannel_theory.html)
and [examples](https://mooseframework.inl.gov/modules/subchannel/v&v/v&v-list.html) for more details.

## Multiphysics Coupling id=couple

### Mesh id=mesh

In this multiphysics model, the heat conduction and neutronics solves use different meshes. The heat conduction
mesh is generated with a standalone input file (`solid_mesh.i`):

!listing /tutorials/rr_lwr_pincell/solid_mesh.i

The pincell is created first with a [PolygonConcentricCircleMeshGenerator](PolygonConcentricCircleMeshGenerator.md),
where the radial regions have the same radii as the source region discretization we set up in the OpenMC [!ac](CSG)
model. This ensures that we maintain a one-to-one mapping between subdomains in the MOOSE mesh and the OpenMC geometry,
and tallies are therefore all normalized by the same volume. The mesh is then extruded axially with an
[AdvancedExtruderGenerator](AdvancedExtruderGenerator.md). We label the boundary between the fuel and the gap
(`fuel_or`), the boundary between the clad and the gap (`clad_ir`), and the boundary between the clad and the
coolant (`clad_or`). These sidesets are used to apply the gap conductance model and the pinned temperature boundary condition
in the heat conduction input file. We then delete the gap and coolant blocks as they are unused in the heat conduction
application. Finally, we scale the mesh such that it is in meters (to match the provided
thermal properties). The resulting mesh can be found in [rr_solid_mesh]. Note that this mesh is coarse for the purpose
of reducing the computational burden in this tutorial, and requires additional refinement to improve temperature
predictions.

!media rr_solid_mesh.png
  id=rr_solid_mesh
  caption=Solid mesh for the heat conduction problem. Red and white: UO@2@ fuel. Green: Zr clad.
  style=width:50%;margin-left:auto;margin-right:auto;halign:center

The OpenMC mesh is used as a transfer target for applying multiphysics feedback to [!ac](CSG) cells, and receiving
fission heating from cell tallies. It is generated in a nearly identical manner to the solid mesh, with a few
key differences: sidesets are not created, and the `gap` and `clad` blocks are deleted because no multiphysics feedback is applied on these blocks (for simplicity). The `fuel` blocks are used to
receive fission heating and apply fuel temperatures, while the `water` block is used to apply coolant densities and
temperatures. The input file used to generate the OpenMC mesh mirror can be found below, and the resulting mesh can be
found in [rr_openmc_mesh].

!listing /tutorials/rr_lwr_pincell/openmc_mesh.i

!media rr_openmc_mesh.png
  id=rr_openmc_mesh
  caption=OpenMC mesh mirror. Red and white: UO@2@ fuel. Teal: water.
  style=width:50%;margin-left:auto;margin-right:auto;halign:center

To generate the meshes, run

```bash
cardinal-opt -i solid_mesh.i --mesh-only
cardinal-opt -i openmc_mesh.i --mesh-only
```

### Neutronics Input File id=openmc_input

The neutronics physics is solved over the entire domain using OpenMC; the OpenMC wrapping is described in `openmc.i`.
We begin by loading the mesh that OpenMC will receive coupling fields from the coupled MOOSE applications, and on which
OpenMC will write the fission heat source:

!listing /tutorials/rr_lwr_pincell/openmc.i
  block=Mesh

Next, we define a series of auxvariables and auxkernels. `cell_temperature` and `cell_density` are used to visualize
the temperatures and densities that Cardinal is applying to OpenMC cells. `average_power` and `linear_power` are used as
storage to compute the linear power required by the subchannel application. `cell_temperature` and `cell_density`
are written to by a [CellTemperatureAux](CellTemperatureAux.md) and [CellDensityAux](CellDensityAux.md) (which
query the cell temperature and density, respectively, in the OpenMC geometry); `average_power` and `linear_power` are written to with a
[SpatialUserObjectAux](SpatialUserObjectAux.md) and [ParsedAux](ParsedAux.md) to unpack an axially-averaged power
density and convert it to a linear power.

!listing /tutorials/rr_lwr_pincell/openmc.i
  start=AuxVariables
  end=Problem

Afterwards, we replace the default finite element problem with an [OpenMCCellAverageProblem](OpenMCCellAverageProblem.md).
This class is responsible for initializing OpenMC, setting cell temperatures and densities (with equivalent field
variables on the `[Mesh]`), and writing results from OpenMC tallies to the MOOSE mesh. We start by overriding the
number of inactive batches, the total number of batches, and the number of particles per batch. It should be noted
that these settings can be applied in `make_openmc_model.py` when generating the OpenMC model as well. A large number of
inactive batches (in this case, 400) are required when running [!ac](TRRM) as both the scattering and fission sources
must be converged. Fewer rays per batch (100 in this case) are required as rays do not terminate until they reach
the end of the active length (unlike particles, which are killed on absorption). We then specify `scaling = 100.0`
to convert from mesh units (m) to OpenMC units (cm). We also set the pin power used to normalize tally scores.
Afterwards, we set the depth in the OpenMC geometry we wish to couple to (`cell_level`), and
specify that we wish to couple OpenMC to the fuel (`uo2` and `uo2_tri` blocks) and coolant (`water` blocks) with
temperature feedback. We also enable density feedback by setting `density_blocks = 'water'`. When applying density
feedback to either the random ray solver or the multi-group Monte Carlo solver in OpenMC, the density used when
computing multi-group cross sections must be provided in `mgxs_reference_densities_by_block` (in units of kg/m$^{3}$)
to rescale multi-group cross sections. To speed up the rate of convergence and avoid the nonlinear feedback
instabilities commonly observed in [!ac](LWR) multiphysics calculations, we use constant relaxation with a
relaxation factor of 0.5. The final component of the OpenMC problem setup is to add a [CellTally](CellTally.md)
which accumulates `kappa_fission` over `uo2` and `uo2_tri`. In addition to fission heating tally value, we
enable outputs for the heating standard deviation and relative error.

!listing /tutorials/rr_lwr_pincell/openmc.i
  block=Problem

The specification of temperature and density blocks in the `[Problem]` will add variables named `temp` and `density`
to the mesh, which the [OpenMCCellAverageProblem](OpenMCCellAverageProblem.md) will query to set cell temperatures and densities.
To zero values of temperature and density being sent to OpenMC, we add a [ConstantIC](ConstantIC.md) which sets these
field variables to the inlet temperature and average fluid density:

!listing /tutorials/rr_lwr_pincell/openmc.i
  block=ICs

The next set of blocks we add sets up the coupling scheme. We add a [TransientMultiApp](TransientMultiApp.md) for
the heat conduction solve (`solid`), and a [FullSolveMultiApp](FullSolveMultiApp.md) for the subchannel solve
(`subchannel`). `solid` receives fission heating from `heat_source_to_solid` (a
[MultiAppGeneralFieldShapeEvaluationTransfer](MultiAppGeneralFieldShapeEvaluationTransfer.md)), the clad surface
temperature from `clad_surface_temperature_to_conduction` (a
[MultiAppGeneralFieldNearestLocationTransfer](MultiAppGeneralFieldNearestLocationTransfer.md)), and sends
the fuel temperature to the main application through `solid_temp_to_openmc` (a
[MultiAppGeneralFieldShapeEvaluationTransfer](MultiAppGeneralFieldShapeEvaluationTransfer.md)). `subchannel`
receives the linear power rate computed by the OpenMC main application through `linear_heat_rate_to_subchannel` (a
[MultiAppGeneralFieldNearestLocationTransfer](MultiAppGeneralFieldNearestLocationTransfer.md)), and sends fluid temperatures
and densities to the OpenMC main application via `fluid_temperature_from_subchannel` and `fluid_density_from_subchannel`
(where both are [MultiAppGeneralFieldNearestLocationTransfer](MultiAppGeneralFieldNearestLocationTransfer.md)). The
subchannel application is executed before OpenMC is run (on `TIMESTEP_BEGIN`), while the  heat conduction
application is executed after running OpenMC (on `TIMESTEP_END`). This results in the following coupling scheme:

1. Run subchannel application (`subchannel`);
2. Send fluid temperatures and densities to OpenMC;
3. Run OpenMC;
4. Send fission power and clad surface temperature to heat conduction application (`solid`);
5. Compute fuel temperatures;
6. Send fuel temperatures back to OpenMC;
7. Repeat until fields are converged.

!listing /tutorials/rr_lwr_pincell/openmc.i
  start=MultiApps
  end=Postprocessors

The final portion of the input file deals with execution, post-processing, and outputs. We use a
[Transient](Transient.md) executioner in this problem to control the number of iterations between
the OpenMC main application and the heat conduction sub-application. In this problem, 5 iterations
are sufficient to converge tally statistics and $k_{eff}$ due to the low variance of [!ac](TRRM).
It should be noted that the problem OpenMC is solving is still a quasi-static k-eigenvalue
calculation, and the heat conduction simulation is a steady-state governing equation with the use of a
[Transient](Transient.md) executioner. We add a series of post-processors to monitor the relative
errors of tally variables, report the k-eigenvalue (and its associated standard deviation), and generate
axial layered averages of various fields for plotting. Finally, we select Exodus and CSV output in the
`[Outputs]` block.

!listing /tutorials/rr_lwr_pincell/openmc.i
  start=Postprocessors

### Solid Input File id=solid_input

The heat conduction problem is setup in the `solid.i` input file, and is very similar to the
[continuous energy LWR pincell tutorial](pincell1.md). It begins by loading the previously
generated mesh.

!listing /tutorials/rr_lwr_pincell/solid.i
  block=Mesh

Afterwards, nonlinear and auxiliary variables are added for the temperature, heat source, and
cladding surface temperature. This is followed by the addition of kernels to assemble the heat
conduction equation over the fuel and cladding (where the fission heat source `heat` is block
restricted to the fuel).

!listing /tutorials/rr_lwr_pincell/solid.i
  start=Variables
  end=Functions

After variables are added, a [MatchedValueBC](MatchedValueBC.md) is specified to force the outer
surface of the cladding to take on the pin surface temperature computed by the subchannel solver:

!listing /tutorials/rr_lwr_pincell/solid.i
  start=BCs
  end=Materials

A series of [GenericConstantMaterial](GenericConstantMaterial.md) are then added to set the
thermal conductivity of the fuel and cladding, which is followed by the addition of a
[GapHeatTransfer](GapHeatTransfer.md) to set up the gap conductance model.

!listing /tutorials/rr_lwr_pincell/solid.i
  start=Materials
  end=Executioner

Finally, the `[Executioner]` is set to [Transient](Transient.md). As no timestepping parameters
are provided, the heat conduction simulation will execute in lock-step with the OpenMC simulation
because `sub_cycling = true` was set in the `openmc.i` file. We add several post-processors to
monitor the maximum fuel and cladding temperatures, and select Exodus and CSV output in the
`[Outputs]` block.

!listing /tutorials/rr_lwr_pincell/solid.i
  start=Executioner

### Subchannel Input File id=subchannel_input

The subchannel problem is setup in the `subchannel.i` input file, and is reasonably similar to the
[continuous energy SFR assembly tutorial](subchannel.md). It begins by creating a 1-D mesh for
each subchannel and fuel pin as mesh storage for fields computed by subchannel and received from
transfers. It should be noted that the subchannel simulation does compute crossflow between channels,
and so it behaves as a simplified 2D solver which is swept in the axial direction. Note that the
subchannel module operates in base SI, and so all spatial quantities must be provided in units of
meters.

!listing /tutorials/rr_lwr_pincell/subchannel.i
  block=Mesh

Next, we define the fluid properties, problem solver settings, and the closure relationships used.
For instance, we set the tolerances for the pressure and temperature solves, and choices for
staggering of the solution of the coupled equations for pressure, flow, and temperature. Two
auxkernels are also used to apply the inlet conditions on temperature and flowrate. A series
of initial conditions are also added to initialize fluid properties, which aides in speeding
up the convergence of the nonlinear problem.

!listing /tutorials/rr_lwr_pincell/subchannel.i
  start=FluidProperties
  end=Executioner

Finally, we specify a steady executioner. We choose to disable output from the subchannel solver
and instead visualize results that have been transferred to the OpenMC main application.

!listing /tutorials/rr_lwr_pincell/subchannel.i
  start=Executioner

## Execution and Postprocessing id=results

To run the coupled calculation with [!ac](TRRM) and flat sources,

```bash
python make_openmc_model.py -r
cardinal-opt -i solid_mesh.i --mesh-only
cardinal-opt -i openmc_mesh.i --mesh-only
mpiexec -np 2 cardinal-opt -i openmc.i --n-threads=4
```

To run with [!ac](TRRM)  and linear sources,

```bash
python make_openmc_model.py -r --linear
cardinal-opt -i solid_mesh.i --mesh-only
cardinal-opt -i openmc_mesh.i --mesh-only
mpiexec -np 2 cardinal-opt -i openmc.i --n-threads=4
```

and to run the continuous energy reference,

```bash
python make_openmc_model.py
cardinal-opt -i solid_mesh.i --mesh-only
cardinal-opt -i openmc_mesh.i --mesh-only
mpiexec -np 2 cardinal-opt -i openmc.i --n-threads=4
```

All of the above will use 4 OpenMP threads and 2 MPI ranks. To run the simulation
faster, you can increase the number of OpenMP threads and/or MPI ranks or decrease the number of particles. Bear
in mind that decreasing the number of particles may result in a failure to converge when running
[!ac](TRRM) as fewer source regions will be hit on each power iteration.

[table2] shows the resulting k-eigenvalues for each [!ac](TRRM) discretization approach compared
against the continuous energy reference (10000 particles per batch, 1100 batches, 100 inactive batches)
on the first Picard iteration when only density feedback is applied. In general, the single physics
[!ac](TRRM) calculations agree reasonably well with the continuous energy reference. The error in the
results is likely caused by the coarse energy group structure, the use of isotropic (transport corrected)
scattering cross sections, and a lack of equivalence methods to better treat U-238 resonance self-shielding.
[table3] shows the k-eigenvalues for each case on the final Picard iteration; the agreement is less than
stellar and can be attributed to the use of isothermal temperature interpolation tables in the multiphysics
model.

!table id=table2 caption=k-eigenvalue predictions on the initial Picard iteration.
| Method | $k_{eff}$ | $k_{CE} - k_{TRRM}$ (PCM) |
| :- | :- | :- |
| CE Monte Carlo | $1.29812 \pm 34 \text{ PCM}$ | N/A |
| Flat Source | $1.29972 \pm 41 \text{ PCM}$ | $−160$ |
| Linear Source | $1.30112 \pm 41 \text{ PCM}$ | $−300$ |

!table id=table3 caption=k-eigenvalue predictions on the final Picard iteration.
| Method | $k_{eff}$ | $k_{CE} - k_{TRRM}$ (PCM) |
| :- | :- | :- |
| CE Monte Carlo | $1.28009 \pm 34 \text{ PCM}$ | N/A |
| Flat Source | $1.26686 \pm 36 \text{ PCM}$ | $1323$ |
| Linear Source | $1.26838 \pm 36 \text{ PCM}$ | $1171$ |

[rr_lwr_radial_comp] plots the center-plane radial fission heating distribution for the different
spatial discretization schemes and compares them with the continuous energy reference on the final
Picard iteration. There is a fairly large discrepancy (up to 5%) which is caused by over-exaggerated
Doppler broadening in the fuel, depressing the pin power compared to the continuous energy
result. We can confirm this trend by plotting the axial centerline power distribution in
[rr_lwr_axial_comparison_power]. The continuous energy result is peaked towards the
subchannel inlet (as expected), while the [!ac](TRRM) calculations are more cosine shaped. This showcases
the need for reasonable [!ac](MGXS) temperature interpolation tables in multiphysics calculations; simple
temperature isotherms will not suffice when there are large thermal gradients with strong temperature feedback.
[rr_lwr_axial_comparison_power] also demonstrates the advantage of [!ac](TRRM) - uniform statistical relative
errors across the problem domain. This is advantageous for whole-core calculations where Monte Carlo transport
struggles to steer particles to the core periphery.

!media rr_lwr_radial_comp.png
  id=rr_lwr_radial_comp
  caption=Comparison of the different [!ac](TRRM) spatial discretization approaches against continuous energy Monte Carlo radially.
  style=width:100%;margin-left:auto;margin-right:auto;halign:center

!media rr_lwr_axial_comparison_power.png
  id=rr_lwr_axial_comparison_power
  caption=Comparison of fission heating (left) and fission heating relative error (right) computed by different [!ac](TRRM) spatial discretization approaches and the continuous energy Monte Carlo reference.
  style=width:100%;margin-left:auto;margin-right:auto;halign:center

Similarily to the fission heating, the fuel temperature ([rr_lwr_axial_comparison_temps] - left) is also cosine
shaped in the [!ac](TRRM) solution while the continuous energy reference is peaked towards the inlet. The coolant
temperatures ([rr_lwr_axial_comparison_temps] - right) and densities ([rr_lwr_axial_comparison_density]) are almost
identical between the different solvers, demonstrating how the subchannel solver is insensitive to the neutronics
discretization for a single pincell.

!media rr_lwr_axial_comparison_temps.png
  id=rr_lwr_axial_comparison_temps
  caption=Comparison of temperatures computed by different [!ac](TRRM) spatial discretization approaches against the continuous energy Monte Carlo reference. Left: fuel temperature. Right: coolant temperature.
  style=width:100%;margin-left:auto;margin-right:auto;halign:center

!media rr_lwr_axial_comparison_density.png
  id=rr_lwr_axial_comparison_density
  caption=Comparison of the density computed by different [!ac](TRRM) spatial discretization approaches against the continuous energy Monte Carlo reference.
  style=width:50%;margin-left:auto;margin-right:auto;halign:center
