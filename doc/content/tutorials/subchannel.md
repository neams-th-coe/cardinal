# SFR Fuel Assembly

In this tutorial, you will learn how to:

- Build a Cardinal model starting from simpler models for each individual physics
- Omit portions of the OpenMC domain from multiphysics feedback
- Couple OpenMC via temperature, density, and heat source feedback to the MOOSE heat conduction and subchannel modules

To access this tutorial,

```
cd cardinal/tutorials/openmc_subchannel
```

The main focus of this tutorial is to teach you how to build a multiphysics simulation step by step.
We first build the "single-physics" inputs for OpenMC, MOOSE heat conduction, and MOOSE subchannel
using placeholder values for various source terms/boundary conditions.
Then, we extend from these files to a full 3-physics coupled case.

## Geometry

This model consists of the active region of the [!ac](CEFR) fuel assembly.
The relevant dimensions are summarized in [table1]. The geometry consists of annular
UO$_2$ fuel pins within a triangular lattice and enclosed in a duct. Sodium coolant
is present outside the cladding.

!table id=table1 caption=Geometric specifications for the [!ac](SFR) fuel assembly
| Parameter | Value (cm) |
| :- | :- |
| Fuel pellet outer diameter | 0.52 |
| Fuel pellet inner diameter | 0.16 |
| Fuel pin pitch | 0.695 |
| Clad inner diameter | 0.54 |
| Clad outer diameter | 0.6 |
| Height of active region | 45.0 |
| Assembly pitch | 6.1 |
| Duct inner flat-to-flat | 5.66 |
| Duct outer flat-to-flat | 5.9 |

The total core power of 65 MWth is assumed uniformly distributed among the 79 fuel bundles,
so that the power of one fuel bundle will be normalized to 0.822 MWth.

In this tutorial, we will only solve for the multiphysics feedback in the interior-bundle region,
i.e. for the sodium and fuel pins inside the duct. The thermal-fluid solution in the duct
and inter-wrapper space (sodium outside the duct) will not be performed in order to illustrate how
an OpenMC model (which does contain these regions) can be coupled to physics feedback over a subset
of its domain. This simplification taken in this tutorial is not a limitation of any of the
physics solvers and is only performed for illustration of a general notion often taken when
building multiphysics models (i.e. usually, not all physics are solved over all parts of the domain).
For instance, it is commonplace to omit the thermal-fluid solution in above-core regions like
axial reflectors because temperature/density changes in these regions have only a small impact on
the neutronics feedback (but these domains have to at least exist in the neutronics model or the
leakage will be very wrong). Any uncoupled regions in OpenMC simply remain at the initial
temperatures and densities in the OpenMC XML files.

The overall approach to build this three-physics coupled model will progress by first:
1. Building single-physics models for MOOSE heat conduction, OpenMC neutronics, and MOOSE subchannel
  with placeholder values for the boundary conditions and source terms required to couple the 3
  physics together
2. Couple OpenMC and heat conduction in the solids
3. Couple OpenMC, heat conduction, and subchannel in the solids and fluids

Our eventual goal will be to accomplish the following multiphysics coupling shown in [full_coupling]
(this is step 3 above).
Red arrows indicate those
data transfers that Cardinal performs, while black arrows indicate [Transfers](Transfers.md)
we will use directly from the MOOSE framework. The mesh shown in the center of the image
is the mesh in the main app's input, and will be the central landing/retrieval point for
the coupling data passed between each of the three coupled solvers.

!media full_coupling.png
  id=full_coupling
  caption=Multiphysics data transfers we will require to couple OpenMC, heat conduction, and subchannel models of the within-duct region of an [!ac](SFR)
  style=width:75%;margin-left:auto;margin-right:auto

Before we progress to this more complicated model, we will first build individual models of
each solver, with all incoming data transfers set to be placeholder values. [single_coupling]
shows each of the single-physics models we will build -- we will not pass data to/from
the centralized transfer mesh at this point, but will simply run each code and demonstrate how
to set placeholder values for the data received by that application. [single_coupling]
represents step 1 above.

!media single_coupling.png
  id=single_coupling
  caption=Three single-physics models we will build to gradually progress to [full_coupling]
  style=width:90%;margin-left:auto;margin-right:auto

Step 2 will then couple OpenMC to MOOSE heat conduction, with transfers shown in [two_coupling].
This will allow us to describe [MultiApps](MultiApps.md) and [Transfers](Transfers.md) with just
two of the physics solvers. Then, we will be ready to progress to the fully coupled case, Step 3,
shown in [full_coupling].

!media single_coupling.png
  id=single_coupling
  caption=Three single-physics models we will build to gradually progress to [full_coupling]
  style=width:90%;margin-left:auto;margin-right:auto

## Step 1: Single-Physics Uncoupled Models

### Heat Conduction Model

First, we will create a heat conduction model for the fuel pins without any multiphysics feedback
to OpenMC or the MOOSE subchannel module. To begin, we build a mesh of the fuel pins
using MOOSE's reactor module for mesh-building. This mesh is built by first creating a
mesh of a single fuel pin, then packing that pin into a hexagonal lattice, and finally by
extruding this 2-D mesh into 3-D.

!listing /tutorials/openmc_subchannel/meshes/mesh.i

To generate the mesh as a file, run the input file in mesh generation mode:

!listing
cardinal-opt -i mesh.i --mesh-only

!media bundle_solid_mesh.png
  id=bundle_solid_mesh
  caption=Mesh created by `mesh.i`
  style=width:75%;margin-left:auto;margin-right:auto

MOOSE mesh generators will often automatically set IDs to the sidesets in the model; in order
to apply our pin surface temperature boundary condition, we need to know the ID of this sideset.
This can be determined by reading the documentation for the various objects in the
`[Mesh]` block, or by visually inspecting the model in Paraview. From Paraview's sideset
panel, we can individually turn sidesets on/off, until we find that sideset 7 is the
pin surface.

!media paraview_sideset.png
  id=paraview_sideset
  caption=Paraview sideset viewer panel can be used to turn sideset views on/off until we find the number of the sideset representing the pin outer surface
  style=width:75%;margin-left:auto;margin-right:auto

Now that we have a mesh, we can create an input file to model heat conduction in the solid
regions, without any multiphysics coupling. These files are located in the `heat_conduction`
sub-folder. The first thing we need to do is actually modify the mesh we just made - you'll
notice that this mesh contains the region pertaining to fluid, which we actually don't need
for the heat conduction solve. This operation is shown by loading the mesh from the file
and then applying another modification to it.

!listing /tutorials/openmc_subchannel/heat_conduction/conduction.i
  block=Mesh

Next, we define the variables and governing equations for the
[heat conduction physics](theory/heat_eqn.md).
We will be solving for temperature, so we first declare a variable named `T`. Then,
we add two kernels, one to represent the diffusive kernel and the second to represent
a volumetric heat source. This heat source will be provided by a coupled variable named
`heat_source` and only exists on block 1, the fuel. The [HeatConduction](HeatConduction.md)
kernel requires specification of a material property named `thermal_conductivity`,
for which we use three [GenericConstantMaterial](GenericConstantMaterial.md) objects,
to set unique values for $k$ on each subdomain in the mesh.

!listing /tutorials/openmc_subchannel/heat_conduction/conduction.i
  start=Variables
  end=AuxVariables

Next, we define auxiliary variables for the fields involved in the coupling - `T_wall` will be
used for the pin outer surface temperature (an input to the conduction model), `heat_source` will be used for
the volumetric heating term (an input to the conduction model), and `q_prime` will be used for the
linear heating rate (an output from the conduction model).
For `T_wall` and `heat_source`, we set a simple initial condition of a constant value.
The choice of 1e8 for the power density is arbitrary here, as we're simply trying to build
one of our input files for a later multiphysics run where this information will come from OpenMC.
Then, we apply a Dirichlet boundary condition, [MatchedValueBC](MatchedValueBC.md), to the
pin outer surface.

!listing /tutorials/openmc_subchannel/heat_conduction/conduction.i
  start=BCs
  end=AuxKernels

For the linear heating rate (W/m), we use a [NearestPointLayeredIntegral](NearestPointLayeredIntegral.md)
to compute an integral in the $z$ direction of the `heat_source` variable, individually
for each fuel pin. This user object is then applied to fill the `q_pime` auxiliary variable
with the [SpatialUserObjectAux](SpatialUserObjectAux.md).

!listing /tutorials/openmc_subchannel/heat_conduction/conduction.i
  start=AuxKernels
  end=Executioner

Finally, we select a [Steady](Steady.md) executioner and write outputs to exodus.

!listing /tutorials/openmc_subchannel/heat_conduction/conduction.i
  start=Executioner

We are now ready to run this model; for example, with 2 MPI ranks and 2 threads per rank,

```
mpiexec -np 2 cardinal-opt -i conduction.i --n-threads=2
```

We create an output file named `conduction_out.e`, which we can open in Paraview and view the
solid temperature (left) and linear heat rate (right). The linear heat rate is constant because
$\dot{q}$ is constant.

!media heat_conduction.png
  id=heat_conduction
  caption=Solid temperature (left) and axial heat rate (right) computed by MOOSE heat conduction model with placeholder values for the pin surface temperature (constant) and power density (constant).

### OpenMC Model

The OpenMC model is built using [!ac](CSG), which are cells created from regions
of space formed by half-spaces of various common surfaces.
OpenMC's Python [!ac](API)
is used to create the pincell model with the script shown below within the `openmc` folder. First, we define
materials and create the geometry. We add create a universe representing a fuel pin, and then repeat that
universe inside of a 3-D [HexLattice](https://docs.openmc.org/en/stable/pythonapi/generated/openmc.HexLattice.html), where the axial layers in the lattice create additional cells
in the axial direction necessary to apply temperature and density feedback with a spatial distribution.
The OpenMC model also contains the duct and sodium interassembly space; these regions will not be
coupled to feedback from MOOSE, so we omit discretizing them in the axial direction for simplicity.

The OpenMC geometry as produced via plots is shown in [bundle_openmc].

!listing /tutorials/openmc_subchannel/openmc/model.py

!media bundle_openmc.png
  id=bundle_openmc
  caption=OpenMC geometry colored by cell ID (left) and material (right) shown on the $x$-$y$ plane
  style=width:60%;margin-left:auto;margin-right:auto

The top and bottom of the domain are vacuum boundaries, while the six lateral faces are reflective.
Because we fill the `fuel_pin_universe` within the entries in the `fuel_pin_lattice`, and we fill
the `fuel_pin_lattice` inside the `hex_WR_IN` surface, applying individual temperatures to each pin
and axial layer will require us to set `lowest_cell_level = 2`.

To generate the XML files needed to run OpenMC, you can run the following:

```
python model.py
```

or simply use the XML files checked in to the `tutorials/openmc_subchannel/openmc` directory.

Now, to run OpenMC via Cardinal we need to create a thin wrapper input file, shown below. We
begin by loading the mesh generated earlier; this mesh is not actually used by OpenMC, but
is where OpenMC will read temperatures/densities from, and to where it will write tallies.

!listing tutorials/openmc_subchannel/openmc/openmc.i
  block=Mesh

Next, we create a block which will override the typical finite element/volume MOOSE solve
by running OpenMC. In this block, we specify the power level we want to normalize the
tallies to (see [here](https://docs.openmc.org/en/stable/usersguide/tallies.html#normalization-of-tally-results))
for more information). Then, we indicate which subdomains in the mesh OpenMC will read temperature
from and read density from. We only read densities for the fluid because any density changes in
the solid would require movement of cell boundaries in order to preserve masses. Then, we finish
by creating a tally over the OpenMC cells, which will automatically get written into
an auxiliary variable named `kappa_fission`.

!listing tutorials/openmc_subchannel/openmc/openmc.i
  block=Problem

The other coupled codes will provide temperatures and densities to OpenMC, but for this single-physics
model we first simply set placeholder values for the temperatures and densities to be read by OpenMC.
We also create two auxiliary variables that will display on the mesh the actual cell temperatures
and densities used in OpenMC; these can be used to confirm that data read from the mesh gets applied correctly
to the OpenMC domain.

!listing tutorials/openmc_subchannel/openmc/openmc.i
  start=ICs
  end=Executioner

Finally, we specify a steady executioner and output results to exodus.

!listing tutorials/openmc_subchannel/openmc/openmc.i
  start=Executioner

To run the model,

```
cardinal-opt -i openmc.i
```

This will generate an output file, `openmc_out.e`, which we can use to visualize the tally.
We note that we see 10 "layers" in the tally in the axial direction, because we've subdivided
the OpenMC model into 10 unique cells in the axial direction.

!media bundle_openmc_q.png
  id=bundle_openmc_q
  caption=OpenMC `kappa-fission` score mapped to the mesh
  style=width:70%;margin-left:auto;margin-right:auto

We can also inspect the `cell_temperature` and `cell_density` variables, and find that they
reflect the temperatures/densities that OpenMC reads from the mesh.

### Subchannel Model

The fluid thermal-fluid model is built using MOOSE's [subchannel module](https://mooseframework.inl.gov/modules/subchannel/index.html). The subchannel module is quite a bit different from a typical MOOSE application,
because the subchannel method is fundamentally different from the finite element/finite volume
methods used elsewhere in MOOSE. The purpose of this tutorial is not to serve as training on the
subchannel module, so we encourage you to explore the [subchannel module examples](https://mooseframework.inl.gov/modules/subchannel/v&v/v&v-list.html)
for more information on how the subchannel solver works. Here, we will only quickly describe the
subchannel input file.

A subchannel method does not use a typical unstructured mesh - rather, the geometry is represented
in a regimented way as a pseudo 2-D geometry stored as internal data structures in the MOOSE
subchannel module. We build a 1-D mesh for each subchannel and fuel pin, just as a retrieval/landing
point to store data computed by subchannel (though it should be understood that the solution from
subchannel does allow crossflows between subchannels, so it is 2-D solver and not a 1-D solver
as the mesh might suggest).

!listing tutorials/openmc_subchannel/subchannel/subchannel.i
  block=TriSubChannelMesh

One auxiliary variable is created for all aspects of the subchannel solution - including
the solution fields temperature (`T`), mass flowrates per channel (`mdot`), and pressure (`P`).
Additional auxiliary variables are used to store the geometry information per channel, such as
the cross-sectional flow areas (`S`) and wetted perimeters (`w_perim`). This block will always
be the same for all subchannel models. We set a constant placeholder value for the linear
heating rate, `q_prime` in lieu of a value to be provided by the heat conduction model.
Additional subchannel-specific initial conditions are needed for some of the internal variables
in the subchannel model; for example, [SCMTriFlowAreaIC](SCMTriFlowAreaIC.md) calculates the
subchannel cross-sectional flow areas for the `S` auxiliary variable.

!listing tutorials/openmc_subchannel/subchannel/subchannel.i
  start=AuxVariables
  end=FluidProperties

Next, we define the fluid properties and the problem solver settings. For instance, we set
the tolerances for the pressure and temperature solves, and choices for staggering of the
solution of the coupled equations for pressure, flow, and temperature. Two auxkernels are
also used to apply the inlet conditions on temperature and flowrate.

!listing tutorials/openmc_subchannel/subchannel/subchannel.i
  start=FluidProperties
  end=Postprocessors

To ensure the model is set up correctly, we can check the global energy conservation -- the
power deposited from the pins into the coolant should equal $\dot{m}C_p\Delta T$ provided that
no heat is lost through the lateral walls of the assembly (true for this problem setup), all changes
to internal energy arise from sensible heat (i.e. no latent heat), the flow is steady-state, and the
pressur work is negligible. We compute the power with a [SCMPowerPostprocessor](SCMPowerPostprocessor.md),
and then the bulk average inlet and outlet temperatures with two [SCMPlanarMean](SCMPlanarMean.md)
postprocessors. The average specific heat is computed at the top of the input file with local
parser syntax based on the correlation used for sodium.

!listing tutorials/openmc_subchannel/subchannel/subchannel.i
  block=Postprocessors

Finally, we specify a steady executioner and will output our results to exodus format. In order
to visualize the subchannel solution in a domain which better represents the subchannel discretization,
we pass a few of the subchannel solution variables to a sub-application in the `viz.i` file, which will
simply render those variables into a format recognizable as subchannel discretization.

!listing tutorials/openmc_subchannel/subchannel/subchannel.i
  start=Executioner

The `viz.i` file does not solve any additional physics, but simply adds a 3-D mesh (not used for any solution)
to visualize the subchannel solution.

!listing tutorials/openmc_subchannel/subchannel/viz.i

We are now ready to run this model:

```
cardinal-opt -i subchannel.i
```

This will create two output files - `subchannel_out.e` which contains the subchannel solution, but shown
on 1-D lines (this is just how subchannel stores its solution internally). The `subchannel_out_viz0.e` file
shows the same data, but in a format representative of the subchannel discretization.

!media subchannel_T.png
  id=subchannel_T
  caption=Subchannel solution for fluid temperature given a constant linear heating rate
  style=width:70%;margin-left:auto;margin-right:auto

We can confirm that this model conserves energy by comparing the value of the `expected_dT` postprocessor
with the difference between the `inlet_temp` and `outlet_temp` postprocessors (the match is
not perfect because we took the specific heat just at a single value, averaged over the bundle
(in other words, we were not 100% precise in the `expression` used in `expected_dT` -- energy is conserved
if we did a more sophisticated integral of $\int\dot{m}C_pTdz$).

## Step 2: Two-Way Multiphysics Coupling
  id=coupling

In this section, two of the codes are coupled together - OpenMC and MOOSE heat conduction in the solids.
The following sub-sections describe these files, with the focus only on those aspects which are now
different to couple two models together.

### Heat Conduction Model

The only change required to the heat conduction model is that we add a postprocessor which will integrate
the power density over the solid mesh to compute the received power from OpenMC - this is required
in case the meshes differ between (i) where the tally is written by OpenMC vs. (ii) the mesh used to
solve for heat conduction, so that energy conservation can be enforced.

!listing tutorials/openmc_subchannel/two_coupled/conduction.i
  block=Postprocessors

We also require the heat conduction model to now use a [Transient](Transient.md) executioner,
because we will be running the model multiple times (due to back and forth coupling with OpenMC).

!listing tutorials/openmc_subchannel/two_coupled/conduction.i
  block=Executioner

Finally, to make the model a little bit more interesting, let's set the fluid temperature to
a linear function from the inlet to the outlet (we will later use a subchannel model to actually
compute this field). For this, we define a function with a linear depencence on height and spanning
between the inlet and outlet temperatures, then apply it to the `T_wall` auxiliary variable.

!listing tutorials/openmc_subchannel/two_coupled/conduction.i
  block=Functions

!listing tutorials/openmc_subchannel/two_coupled/conduction.i
  block=AuxKernels

### OpenMC Model

No changes are required to the XML files; we will simply add the [MultiApps](MultiApps.md)
and [Transfers](Transfers.md) to communicate data between OpenMC and the heat conduction
model. We can select either OpenMC or MOOSE to be the main, controlling application - here,
we select OpenMC.

The only changes we require to the OpenMC input file are to (i) switch to a [Transient](Transient.md)
executioner and to add the [MultiApps](MultiApps.md) and [Transfers](Transfers.md) blocks.
We register a multiapp to solve the heat conduction physics. Then, we add two data transfers
to represent the data received/computed by OpenMC to couple with heat conduction. These
transfers will pass mesh-based data using a nearest-node transfer. First, OpenMC will pass
the power distribution to the solid, making sure to conserve power by normalizing the
received `heat_source` in the `conduction.i` file by the ratio of the power integral on the
source and receiving mesh. The second transfer retrieves solid temperatures from the conduction
model and applies them to the solid blocks in the mesh. By using a transient executioner, we can
require the model to run for a fixed number of Picard iterations (5, in this example).
Automated Picard convergence options, such as [steady-state detection](https://mooseframework.inl.gov/source/executioners/Transient.html#d958c0de-a586-40de-a488-0e497cf7d1ae), are recommended for production calculations.

!listing tutorials/openmc_subchannel/two_coupled/openmc.i
  start=Executioner

### Running

We can now run the two-physics coupled model. For example, with 2 MPI ranks and 3 threads per rank,

```
mpiexec -np 3 cardinal-opt -i openmc.i
```

This will create two output files - `openmc_out.e`, which contains all the variables defined in the
`openmc.i` file; and `openmc_out_conduction0.e`, which will contain all the variables defined
in the `conduction.i` file. Inspecting the files, we can compare the `cell_temperature` in the
`openmc_out.e`, which shows the actual temperatures applied to the OpenMC cells, against the
continuous finite element temperature field solved by the heat conduction solver
(`T` in `openmc_out_conduction0.e`). In [two_coupled], a comparison between these two fields
is shown on the axial midplane of the fuel (left). The continuous finite element conduction solution
is shown on the right.

!media two_coupled.png
  id=two_coupled
  caption=Temperature solution fields for the two-way coupled simulation
  style=width:85%;margin-left:auto;margin-right:auto

## Step 3: Three-Way Multiphysics Coupling

For our last simulation, we couple all three solvers - OpenMC, MOOSE heat conduction, and MOOSE
subchannel. To describe this model, we will only describe what is now different from the two-way
coupling described previously.

### Heat Conduction Model

The heat conduction model is the same as that for the two-way coupling, except we remove the
function we applied for the cladding surface temperature (it will now be passed into the
`conduction.i` file from the subchannel application). We can also delete the `viz.i` multiapp
which was used just to display data, because we will map the subchannel solution onto the mesh
in the `openmc.i` file, where it will be straightforward to visualize the subchannel solution.
The entire file is below.

!listing tutorials/openmc_subchannel/all_coupled/conduction.i

### Subchannel Model

The subchannel model is the same as that for the single-physics model developed earlier.
The linear heat rate will be passed into the `subchannel.i` file by the heat conduction solver.
The entire file is below.

!listing tutorials/openmc_subchannel/all_coupled/subchannel.i

### OpenMC Model

The OpenMC model is the main application which controls the multiphysics solution. We
add a [FullSolveMultiApp](FullSolveMultiApp.md) for the subchannel solver, which will run an entire
subchannel simulation each time the subchannel solver is executed. Then, we add four additional transfers,
to pass (i) $q'$ from the conduction solve to the subchannel solver, (ii-iii) retrieve the fluid density
and temperature from the subchannel solver, and (iv) pass the clad surface temperature to the conduction
model. The entire file is below.

!listing tutorials/openmc_subchannel/all_coupled/openmc.i

### Running

To run the three-physics model using 2 MPI ranks and 4 threads per rank (just an example),

```
mpiexec -np 2 cardinal-opt -i openmc.i --n-threads=4
```

This will create three output files; `openmc_out.e` contains the variables from the `openmc.i` file,
`openmc_out_conduction0.e` contains the variables from the `conduction.i` file, and
`openmc_out_subchannel0.e` contains the variables from the `subchannel.i` file.

[cell_vs_subchannel] shows the subchannel fluid temperature solution (left) and
the cell temperatures applied to the OpenMC model (right). Note how you can see
the typical subchannel discretization from the subchannel solver, but that because
the OpenMC cells are pin-centered representations in a hexagonal lattice, the volume-averaged
temperatures taken from the subchannel solver are applied to the cells.
Note that the cell temperatures in OpenMC, as shown in the image below, do not necessarily
look identical to the OpenMC cells because we're only seeing the "shadow" or projection
of the OpenMC cell temperatures onto a mesh, whose elements may not conform to the
OpenMC cell boundaries.

!media cells_vs_subchannel.png
  id=cells_vs_subchannel
  caption=Temperature computed by the subchannel solver (left) as compared to the cell temperatures applied in OpenMC (right)
  style=width:75%;margin-left:auto;margin-right:auto

The power distribution in the lateral direction is relatively flat, but shows an
cosine-like shape in the axial direction due to the leakage on the top and bottom
of the assembly. This power distribution is applied to the heat conduction model,
resulting in a similar temperature distribution in the fuel (though not identical,
as the pin surface temperature follows a typical "s-curve" in the vertical direction
as the coolant heats up).

!media sfr_power_temp.png
  id=sfr_power_temp
  caption=Power computed by OpenMC (top) and the fuel temperature distribution computed by MOOSE (bottom)
  style=width:65%;margin-left:auto;margin-right:auto


You can try several modifications to these files to improve the resolution of the feedback:

- Subdivide the model into additional axial layers
- Subdivide the OpenMC cells in the fuel into radial rings, to capture radial temperature
  variations within the fuel pins
- Try a [MeshTally](MeshTally.md) for the fuel heating, or use a different score such as `heating_local` to also compute heat deposited in non-fuel regions

