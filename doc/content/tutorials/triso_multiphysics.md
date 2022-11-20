# Multiphysics for a TRISO Gas-Cooled Compact

In this tutorial, you will learn how to:

- Couple OpenMC, NekRS/THM, and MOOSE together for multiphysics modeling of a [!ac](TRISO) compact
- Use two different MultiApp hierarchies to achieve different data transfers
- Use triggers to automatically terminate the OpenMC active batches once reaching the desired statistical uncertainty
- Automatically detect steady state

To access this tutorial,

```
cd cardinal/tutorials/gas_compact_multiphysics
```

This tutorial also requires you to download mesh files and a NekRS
restart file from Box. Please download the files from the `gas_compact_multiphysics`
folder [here](https://anl.app.box.com/s/irryqrx97n5vi4jmct1e3roqgmhzic89) and place
these within the same directory structure in `tutorials/gas_compact_multiphysics`.

!alert! note title=Computing Needs
This tutorial requires [!ac](HPC) resources to run. Please still read this tutorial
if you do not have resources, because you will apply some concepts from this tutorial
in [Tutorial 8B](pincell_multiphysics.md), which you can run on a typical personal
computer.
!alert-end!

In this tutorial, we couple OpenMC to the MOOSE heat conduction module, with fluid
feedback provided by *either* NekRS or THM,

- NekRS: we solve the wall-resolved $k$-\$tau$ [!ac](RANS) equations
- THM: we solve the 1-D area-averaged Navier-Stokes equations using the
  [Thermal Hydraulics Module (THM)](https://mooseframework.inl.gov/modules/thermal_hydraulics/index.html)

Two different multiapp hierarchies will be used in order
to demonstrate the flexibility of the [MultiApp](https://mooseframework.inl.gov/syntax/MultiApps/index.html)
system. The same OpenMC model can be used to provide
feedback to different combinations of MOOSE applications.

In this tutorial, OpenMC receives temperature feedback from the MOOSE heat
conduction module (for the solid regions) and NekRS/THM (for the fluid regions). Density
feedback is provided by NekRS/THM for the fluid regions. This tutorial models a
partial-height [!ac](TRISO)-fueled unit cell of a prismatic gas reactor assembly, and is
a continuation of the [!ac](CHT) [Tutorial 2C](https://cardinal.cels.anl.gov/tutorials/cht3.html).

This tutorial was developed with support from the NEAMS Thermal Fluids Center
of Excellence and is described in more detail in our journal article [!cite](novak2022_cardinal).

## Geometry and Computational Model

The geometry consists of a [!ac](TRISO)-fueled
gas reactor compact unit cell [!cite](sterbentz).
A top-down view of the geometry is shown in
[unit_cell]. The fuel is cooled by helium flowing in a cylindrical channel
of diameter $d_c$. Cylindrical fuel compacts containing randomly-dispersed
[!ac](TRISO) particles at 15% packing fraction
are arranged around the coolant channel in a triangular
lattice.
The [!ac](TRISO) particles use a conventional design that consists of a central
fissile uranium oxycarbide kernel enclosed in a carbon buffer, an inner
[!ac](PyC) layer, a silicon carbide layer, and finally an outer
[!ac](PyC) layer. The geometric specifications are summarized in [table1].
Heat is produced in the [!ac](TRISO) particles to yield a total power of 38 kW.

!media compact_unit_cell.png
  id=unit_cell
  caption=[!ac](TRISO)-fueled gas reactor compact unit cell
  style=width:60%;margin-left:auto;margin-right:auto

!table id=table1 caption=Geometric specifications for a [!ac](TRISO)-fueled gas reactor compact
| Parameter | Value (cm) |
| :- | :- |
| Coolant channel diameter, $d_c$ | 1.6 |
| Fuel compact diameter, $d_f$ | 1.27 |
| Fuel-to-coolant center distance, $p_{cf}$ | 1.628 |
| Height | 160 |
| TRISO kernel radius | 214.85e-4 |
| Buffer layer radius | 314.85e-4 |
| Inner PyC layer radius | 354.85e-4 |
| Silicon carbide layer radius | 389.85e-4 |
| Outer PyC layer radius | 429.85e-4 |

Two different [MultiApp](https://mooseframework.inl.gov/syntax/MultiApps/index.html)
hierarchies are used in this tutorial:

- A "single stack" design where each application
  has either a single "parent" application or "child" application
- A "tree" design where each application has a single "parent" application,
  but multiple "child" applications

[multiapps] shows a conceptual depiction of these hierarchies.
We will describe these in greater detail later,
but introduce them here to assist with describing
a few aspects of the single-physics models. The circled
numbers indicate the order in which the applications run.

Solid lines
depict transfers that occur directly from application $A$ to application $B$, or between
the source and receiver of that field. Dashed lines, on the other hand, depict transfers
that do not occur directly between the source and receiver of the field - for instance,
in the shown "single stack" hierarchy, the NekRS application can only communicate data
with it's immediate parent application. Therefore, to send fluid density and temperature from
NekRS to OpenMC, there are actually two transfers - 1) sending fluid density and temperature from
NekRS to the MOOSE heat conduction module, and 2) sending fluid density and temperature from
the MOOSE heat conduction module to OpenMC.

!media multiapp_options.png
  id=multiapps
  caption=MultiApp hierarchies used in this tutorial; data transfers are shown with solid and dashed lines. Solid lines indicate transfers that occur directly from application A to application B, while dashed lines show transfers that have to first pass through an intermediate application to get to the eventual target application.
  style=width:80%;margin-left:auto;margin-right:auto

Conversely, with the "tree" hierarhcy, MOOSE MultiApps can currently only
communicate with parent/child applications -- *not* sibling applications.
Therefore, all data communicated between the MOOSE heat conduction module and
THM actually has to first pass through their common parent application before reaching
the desired target application.

### OpenMC Model

The OpenMC model is built using [!ac](CSG). The [!ac](TRISO) positions are
sampled using the [!ac](RSA) [algorithm in OpenMC](https://docs.openmc.org/en/stable/examples/triso.html).
OpenMC's Python [!ac](API) is
used to create the model with the script shown below. First, we define materials.
Next, we create a single [!ac](TRISO) particle universe
consisting of the five layers of the particle and an infinite extent of graphite
filling all other space. We then pack pack uniform-radius spheres into a cylindrical
region representing a fuel compact, setting each sphere to be filled with the
[!ac](TRISO) universe.

!listing /tutorials/gas_compact_multiphysics/unit_cell.py language=python

Finally, we loop over
$n_l$ axial layers and create unique cells for each of the six compacts, the graphite
block, and the coolant. Recall that we need unique cells in order for each region to obtain a
a unique temperature from MOOSE. The level on which we will apply
feedback from MOOSE is set to 1 because each layer is a component in a lattice nested once
with respect to the highest level. To accelerate the particle tracking, we:

- Repeat the same [!ac](TRISO) universe in each axial layer and within each compact
- Superimpose a Cartesian search lattice in the fuel channel regions.

The OpenMC geometry, colored by cell ID,
is shown in [openmc_model].
The lateral faces of the unit cell are periodic, while the top
and bottom boundaries are vacuum. The Cartesian search lattice in the fuel compact
regions is also visible.

!media compact_cells.png
  id=openmc_model
  caption=OpenMC model, colored by cell ID
  style=width:60%;margin-left:auto;margin-right:auto

For the "single-stack" MultiApp hierarchy, OpenMC runs first, so the initial
temperature is set to uniform in the radial direction and given by a linear variation
between the inlet and outlet fluid temperatures. The fluid density is then set using
the ideal gas [!ac](EOS) with pressure taken as the fixed outlet of 7.1 MPa given the
temperature, i.e. $\rho_f(P, T)$. For the "tree" MultiApp hierarchy, OpenMC insteady runs
after the MOOSE heat conduction module, but before THM. For this structure, initial
conditions are only required for fluid temperature and density, which are taken as the
same initial conditions as for the "single-stack" case.

To create the XML files required to run OpenMC, run the script:

```
python unit_cell.py
```

You can also use the XML files checked in to the `tutorials/gas_compact_multiphysics` directory.

### Heat Conduction Model

The MOOSE heat conduction module is used to solve for [energy conservation in the solid](theory/heat_eqn.md).
The solid mesh is shown in [solid_mesh].
The [!ac](TRISO) particles are homogenized into
the compact regions - all material properties in the heterogeneous regions
are taken as volume averages of the various constituent materials.
To simplify the specification of
material properties, the solid geometry uses a length unit of meters.

!media compact_solid_mesh.png
  id=solid_mesh
  caption=Mesh for the solid heat conduction model
  style=width:60%;margin-left:auto;margin-right:auto

On the coolant channel surface, a Dirichlet temperature is provided by NekRS/THM.
All other boundaries are insulated. The volumetric power density is provided by OpenMC,
with normalization to ensure the total specified power. When using the "single stack"
hierarchy, MOOSE runs after OpenMC but before NekRS, and an initial condition is only
required for the wall temperature, which is set to a linear variation from inlet to
outlet fluid temperature. When using the "tree" hierarchy, MOOSE runs first, in which
case the initial wall temperature is taken as the same linear variation, while the
power is taken as uniform.

### NekRS Model

NekRS is used to solve the [incompressible k-tau RANS model](theory/ktau.md).
The inlet mass flowrate is 0.0905 kg/s; with the channel diameter of 1.6 cm and material
properties of helium, this results in a Reynolds number of 223214 and a Prandtl number
of 0.655. This highly-turbulent flow results in extremely thin momentum and thermal boundary
layers on the no-slip surfaces forming the periphery of the coolant channel. In order to
resolve the near-wall behavior with a wall-resolved model, an extremely fine mesh is
required in the NekRS simulation. To accelerate the overall coupled solve
that is of interest in this tutorial, the NekRS model is split into a series of calculations:

1. We first run a partial-height, periodic flow-only case
   to obtain converged $P$, $\vec{u}$, and $\mu_T$ distributions.
2. Then, we extrapolate the $\vec{u}$ and $\mu_T$ to the full-height case.
3. We use the converged, full-height $\vec{u}$ and $\mu_T$ distributions
   to transport a temperature passive scalar in a [!ac](CHT) calculation with MOOSE.
4. Finally, we use the converged [!ac](CHT) case as an initial condition for the multiphysics
   simulation with OpenMC and MOOSE feedback.

Steps 1-3 were performed in [Tutorial 2C](https://cardinal.cels.anl.gov/tutorials/cht3.html) -
for brevity, we skip repeating the discussion of steps 1-3.

For the multiphysics case, we will load the restart file produced from step 3, compute $k_T$ from the
loaded solutions for $k$ and $\tau$, and then transport temperature with coupling to MOOSE
heat conduction and OpenMC particle transport.
Let's now describe the NekRS input files needed for the passive scalar solve:

- `ranstube.re2`: NekRS mesh
- `ranstube.par`: High-level settings for the solver, boundary condition mappings to sidesets, and the equations to solve
- `ranstube.udf`: User-defined C++ functions for on-line postprocessing and model setup
- `ranstube.oudf`: User-defined [!ac](OCCA) kernels for boundary conditions and source terms

A detailed description of all of the available parameters, settings, and use
cases for these input files is available on the
[NekRS documentation website](https://nekrsdoc.readthedocs.io/en/latest/index.html).
Because the purpose of this analysis is to demonstrate Cardinal's capabilities, only the aspects
of NekRS required to understand the present case will be covered. First, the NekRS mesh
is shown in [nek_mesh]. Boundary 1 is the inlet, boundary 2 is the outlet, and boundary
3 is the wall. The same mesh was used for the periodic flow solve, except with
a shorter height.

!media nek_mesh_uc.png
  id=nek_mesh
  caption=Mesh for the NekRS [!ac](RANS) model
  style=width:60%;margin-left:auto;margin-right:auto

Next, the `.par` file contains problem setup information.
This input sets up a nondimensional passive scalar solution, loading $P$, $\vec{u}$,
$k$, and $\tau$ from a restart file. We "freeze" the flow by setting
`solver = none` in the `[VELOCITY]`, `[SCALAR01]` ($k$ passive scalar),
and `[SCALAR02]` ($\tau$ passive scalar) blocks. In the nondimensional formulation,
the "viscosity" becomes $1/Re$, where $Re$ is the Reynolds number, while the
"thermal conductivity" becomes $1/Pe$, where $Pe$ is the Peclet number. These nondimensional
numbers are used to set various diffusion coefficients in the governing equations
with syntax like `-223214`, which is equivalent in NekRS syntax to $\frac{1}{223214}$.
The only equation that NekRS will solve is for temperature.

!listing /tutorials/gas_compact_multiphysics/ranstube.par

Next, the `.udf` file is used to setup initial conditions and define how
$k_T$ should be computed based on $Pr_T$ and the restart values of $k$ and $\tau$.
In `turbulent_props`, a user-defined function, we use $k_f$ from the input file
in combination with the $Pr_T$ and $\mu_T$ (read from the restart file later in
the `.udf` file) to adjust the total diffusion coefficient on temperature to
$k_f+k_T$ according to the [turbulent Prandtl number definition](theory/ktau.md). This adjustment must happen on device, in a new GPU kernel we name
`scalarScaledAddKernel`. This kernel will be defined in the `.oudf` file; we
instruct the JIT compilation to compile this new kernel by calling
`udfBuildKernel`.

Then, in `UDF_Setup` we store the value of $\mu_T$ computed in the
restart file.

!listing /tutorials/gas_compact_multiphysics/ranstube.udf language=cpp

In the `.oudf` file, we define boundary conditions for temperature and also
the form of the `scalarScaledAdd` kernel that we use to compute $k_T$.
The inlet boundary is set to a temperature of 0 (a dimensional temperature of
$T_{ref}$), while the fluid-solid interface will receive a heat flux from MOOSE.

!listing /tutorials/gas_compact_multiphysics/ranstube.oudf language=cpp

For this tutorial, NekRS runs last in the "single-stack" MultiApp hierarchy,
so no initial conditions are required aside from the $T$, $\vec{u}$,
and $\mu_T$ taken from the `converged_cht.fld` restart file on Box.

### THM Model

THM is used to solve the [1-D area-averaged Navier-Stokes equations](theory/thm.md).
The THM mesh contains 150 elements; the mesh is constucted automatically
within THM. To simplify the specification of material properties, the fluid geometry
uses a length unit of meters. The heat flux imposed in the THM elements is obtained
by area averaging the heat flux from the heat conduction model in 150 layers along
the fluid-solid interface. For the reverse transfer, the wall temperature sent to MOOSE
heat conduction is set to a uniform value along the fluid-solid interface according to
a nearest-node mapping to the THM elements.

For this tutorial, THM runs last in the "tree" MultiApp hierarchy; because THM solves
time-dependent equations, initial conditions are only required for the solution variables
for which THM solves - pressure, fluid temperature, and velocity, all of which are set
to uniform conditions.

## Multiphysics Coupling

In this section, OpenMC, NekRS/THM, and MOOSE heat conduction are coupled for multiphysics
modeling of the [!ac](TRISO) gas compact.
Two separate simulations are performed here:

- Coupling of OpenMC, NekRS, and MOOSE heat conduction in a "single-stack" MultiApp hierarchy
- Coupling of OpenMC, THM, a MOOSE heat conduction in a "tree" MultiApp hierarchy

By individually describing the two setups, you will understand the customizability
of the MultiApp system and the flexibility shared by all MOOSE applications for seamlessly
exchanging tools of varying resolution for one another.

### OpenMC-NekRS-MOOSE

In this section, we describe the coupling of OpenMC, NekRS, and MOOSE
in the "single-stack" MultiApp hierarchy shown in [multiapps].

#### OpenMC Input Files

The neutronics physics is solved over the entire domain with OpenMC.
The OpenMC wrapping used for the OpenMC-NekRS-MOOSE coupling is described in
the `openm_nek.i` input file.
We begin by defining a number of constants
and by setting up the mesh mirror on which OpenMC will receive temperature
and density from THM-MOOSE, and on which OpenMC will write the fission heat
source. Because the coupled applications use length units of meters, the mesh mirror
must also be in units of meters in order to obtained correct data transfers.
For simplicity, the solid regions use the same mesh as used for solving the
solid heat conduction; for the fluid regions, we use MOOSE
[MeshGenerators](https://mooseframework.inl.gov/syntax/Mesh/index.html)
to form a 2-D circular disk and extrude it into the $z$ direction.

!listing /tutorials/gas_compact_multiphysics/openmc_nek.i
  end=AuxVariables

Next, we define a number of auxiliary variables to query the OpenMC solution
and set up multiphysics coupling. We add `cell_temperature` and
`cell_density` in order to read the cell temperatures and densities directly
from OpenMC in order to visualize the temperatures and densities ultimately applied
to OpenMC's cells. Recall that [OpenMCCellAverageProblem](https://cardinal.cels.anl.gov/source/problems/OpenMCCellAverageProblem.html)
automatically adds variables named `temp`, `density`, and `heat_source` to receive
the temperature and density *sent* to OpenMC and the heat source *extracted*
from OpenMC. In order to compute density using the ideal gas [!ac](EOS) given a temperature
and a fixed pressure, we use a [FluidDensityAux](https://mooseframework.inl.gov/source/auxkernels/FluidDensityAux.html)
to set density.

!listing /tutorials/gas_compact_multiphysics/openmc_nek.i
  start=AuxVariables
  end=ICs

Next, we define the necessary initial conditions using functions; all temperatures
(`nek_temp` and `solid_temp` are to be discussed shortly) are set to a linear variation
from the inlet to outlet fluid temperatures.

!listing /tutorials/gas_compact_multiphysics/openmc_nek.i
  start=ICs
  end=Problem

The wrapping of OpenMC is specified in the `[Problem]` block. Here, we
indicate that we will provide both fluid and solid feedback to OpenMC. In order to
visualize the tally standard deviation, we output the fission tally standard
deviation using the `output` parameter. The heat source from OpenMC will be relaxed
using Robbins-Monro relaxation. Because two different applications are providing
temperatures to OpenMC, we need to "collate" those temperatures into the single
`temp` variable that OpenMC reads from (otherwise, MOOSE will write into `temp`,
which would just be overwritten by NekRS writing into `temp` later in the Picard
step). Cardinal contains convenient syntax to automatically set up the necessary
receiver variables and the auxiliary kernels to write into `temp`, by using
the `temperature_variables` and `temperature_blocks` parameters.

Finally, a number of "triggers" are used to automatically terminate OpenMC's
active batches once reaching desired uncertainties in $k$ and the fission tally.
The number of batches here is terminated once both of the following are satisfied:

- Standard deviation in $k$ is less than 75 pcm
- Maximum fission tally relative error is less than 1%

These criteria are checked every `batch_interval`, up to a maximum number of batches.

!listing /tutorials/gas_compact_multiphysics/openmc_nek.i
  block=Problem

Next, we define a transient executioner - while OpenMC is technically solving
a steady $k$-eigenvalue problem, using a time-dependent executioner with the notion
of a "time step" will allow us to control the frequency with which OpenMC sends data
to/from it's sub-app (MOOSE heat conduction). Here, we set the time step to be 1000 times
the NekRS fluid time step. We will terminate the solution once reaching the specified
`steady_state_tolerance`, which by setting `check_aux = true` will terminate the entire
solution once reaching less than 1% change in all auxiliary variables in the OpenMC
input file (which includes temperatures and power).

!listing /tutorials/gas_compact_multiphysics/openmc_nek.i
  block=Executioner

Next, we define the MOOSE heat conduction sub-application and data transfers to/from
that application. Most important to note is that while the MOOSE heat conduction module
does not itself compute fluid temperature, we see a transfer getting the fluid temperature
that has been transferred to the MOOSE heat conduction module by the doubly-nested NekRS
sub-application. Note that the temperature transfers do *not* write straight into `temp`,
but instead into the scratch space variables we set up with the `temperature_variables`
parameter of [OpenMCCellAverageProblem](https://cardinal.cels.anl.gov/source/problems/OpenMCCellAverageProblem.html).

!listing /tutorials/gas_compact_multiphysics/openmc_nek.i
  start=MultiApp
  end=Postprocessors

Next, we define several postprocessors for querying the solution. The `heat_source`
postprocessor will be used to ensure conservation of power when sent to the MOOSE
heat conduction application. All other postprocessors are used for general solution monitoring
purposes.

!listing /tutorials/gas_compact_multiphysics/openmc_nek.i
  block=Postprocessors

For postprocessing, we also compute the average power distribution in a number of layers
using a [LayeredAverage](https://mooseframework.inl.gov/source/userobject/LayeredAverage.html)
and output to CSV using a [SpatialUserObjectVectorPostprocessor](https://mooseframework.inl.gov/source/vectorpostprocessors/SpatialUserObjectVectorPostprocessor.html),
in combination with a CSV output.

!listing /tutorials/gas_compact_multiphysics/openmc_nek.i
  start=UserObjects

#### Solid Input Files

The solid heat conduction physics is solved over the solid regions of the unit cell
using the MOOSE heat conduction module. The input file for this portion of the physics is the
`solid_nek.i` input. We begin by defining a number of constants and by setting up
the mesh for solving heat conduction. Note that a mesh region is added to represent the
coolant channel, *even though no solve occurs on it*. The coolant region must be meshed
so that NekRS, a sub-application to this solid application, can send a coolant temperature
somewhere so that the OpenMC parent application has somewhere to read from.

!listing /tutorials/gas_compact_multiphysics/solid_nek.i
  end=Problem

Because we have a block in the problem that we don't need to define any material
properties on, we technically need to turn of a material coverage check, or else we're
going to get an error from MOOSE. [FEProblem](https://mooseframework.inl.gov/source/problems/FEProblem.html)
is just the default problem, which we need to list in order to turn off the
material coverage check.

!listing /tutorials/gas_compact_multiphysics/solid_nek.i
  block=Problem

Next, we define the nonlinear variable that this application will solve for - `T`,
the solid temperature. On the solid blocks, we solve the heat equation, but
on the fluid blocks that exist exclusively for transferring data, we add a
[NullKernel](https://mooseframework.inl.gov/source/kernels/NullKernel.html)
to essentially skip the solve in those regions. On the channel wall, the temperature from
NekRS is applied as a Dirichlet condition.

!listing /tutorials/gas_compact_multiphysics/solid_nek.i
  start=Variables
  end=Functions

Next, we define a number of functions to set the solid material properties and define
an initial condition for the wall temperature. The solid material properties are
then applied with a [HeatConductionMaterial](https://mooseframework.inl.gov/source/materials/HeatConductionMaterial.html).

!listing /tutorials/gas_compact_multiphysics/solid_nek.i
  start=ICs
  end=Postprocessors

Next, we define a number of auxiliary variables - `power` will receive the
heat source from OpenMC, `nek_temp` will receive the *wall* temperature from NekRS,
`nek_bulk_temp` will receive the volumetric fluid temperature from NekRS, and
finally `flux` will be used to compute the wall heat flux to send to NekRS.

!listing /tutorials/gas_compact_multiphysics/solid_nek.i
  start=AuxVariables
  end=Postprocessors

We then add a NekRS sub-application and define the transfers to/from NekRS, *including*
the transfer of fluid temperature from the NekRS volume to the dummy coolant
blocks in the MOOSE solid model. Because
the NekRS mesh mirror will be a volume mirror (in order to extract volumetric temperatures
for the neutronics feedback), a significant cost savings can be obtained by using
the "minimal transfer" feature of [NekRSProblem](https://cardinal.cels.anl.gov/source/problems/NekRSProblem.html)
(which requires sending a dummy [Receiver](https://mooseframework.inl.gov/source/postprocessors/Receiver.html)
postprocessor, here named `synchronization_to_nek`, to indicate when data is to be exchanged).
For more information, please consult the documentation for
[NekRSProblem](https://cardinal.cels.anl.gov/source/problems/NekRSProblem.html).

!listing /tutorials/gas_compact_multiphysics/solid_nek.i
  start=MultiApps
  end=Executioner

We add several postprocessors to facilitate the data transfers as well as to query the
solution.

!listing /tutorials/gas_compact_multiphysics/solid_nek.i
  block=Postprocessors

We add a transient executioner - again, even though the MOOSE heat conduction
module in this tutorial solves steady equations, a transient executioner allows us
to control the frequency with which MOOSE and NekRS iterate the [!ac](CHT) physics.
Here, we use a time step that is 50 times bigger than the NekRS time step.

!listing /tutorials/gas_compact_multiphysics/solid_nek.i
  block=Executioner

!alert note
The ability to use different data transfer "frequencies" is a key advantage of the
"single-stack" MultiApp hierarchy. So far, we have shown that OpenMC will exchange
data every 1000 time steps with MOOSE, but MOOSE exchanges data every 50 times steps
with NekRS. In other words, for every update of the OpenMC fission distribution, we
perform 20 sub-iterations of the [!ac](CHT) physics. Depending on the problem,
begin able to iterate the thermal-fluid physics on a finer granularity than the
neutronics feedback can be essential to obtaining a stable solution without an inordinately
high number of Monte Carlo solves.

Finally, we add a number of [LayeredAverage](https://mooseframework.inl.gov/source/userobject/LayeredAverage.html)
user objects to compute averages of the fuel and graphite temperatures in the axial direction,
which are output to CSV using a
[SpatialUserObjectVectorPostprocessor](https://mooseframework.inl.gov/source/vectorpostprocessors/SpatialUserObjectVectorPostprocessor.html),
in combination with a CSV output.

!listing /tutorials/gas_compact_multiphysics/solid_nek.i
  start=UserObjects

#### Fluid Input Files

The fluid mass, momentum, and energy transport physics are solved using NekRS. The input
file for this portion of the physics is the `nek.i` input. We begin by defining a number of constants
and by setting up the [NekRSMesh](https://cardinal.cels.anl.gov/source/mesh/NekRSMesh.html)
mesh mirror. Because we are coupling via boundary [!ac](CHT) to MOOSE, we
set `boundary = '3'` so that we will be able to extract the boundary temperature
from boundary 3 (the wall). We are *also* coupling via volumes to OpenMC higher
in the MultiApp hierarchy. In order to extract volume representations of the
fluid temperature, we also set `volume = true`.

!listing /tutorials/gas_compact_multiphysics/nek.i
  end=Problem

The bulk of the NekRS wrapping occurs in the `[Problem]` block
with [NekRSProblem](https://cardinal.cels.anl.gov/source/problems/NekRSProblem.html).
The NekRS input files are in non-dimensional form, whereas all other coupled applications
use dimensional units. The various `*_ref` and `*_0` parameters define the characteristic
scales that were used to non-dimensionalize the NekRS input. In order to simplify the input
file, we know a priori that OpenMC will not be sending a heat source *to NekRS*, so
we set `has_heat_source = false` so that we don't need to add a dummy heat
source kernel to the `ranstube.oudf` file. Finally, we indicate that we will
be minimizing the data transfers in/out of NekRS unless new data is actually available
from the MOOSE heat conduction module with the `synchronization_interval = parent_app` parameter.

!listing /tutorials/gas_compact_multiphysics/nek.i
  block=Problem

Next, we will allow NekRS to select its own time step using the [NekTimeStepper](https://cardinal.cels.anl.gov/source/timesteppers/NekTimeStepper.html), combined with a transient executioner.

!listing /tutorials/gas_compact_multiphysics/nek.i
  block=Executioner

We also add a number of postprocessors to query the Nek solution.

!listing /tutorials/gas_compact_multiphysics/nek.i
  block=Postprocessors

Finally, we define the output formats and hide the automatically-created
`flux_integral` and `transfer_in` postprocessors from the screen (console)
to have neater output.

!listing /tutorials/gas_compact_multiphysics/nek.i
  block=Outputs

### OpenMC-THM-MOOSE

In this section, we describe the coupling of OpenMC, THM, and MOOSE
in the "tree" MultiApp hierarchy shown in [multiapps]. For the most part,
the OpenMC and MOOSE heat conduction input files only have small differences
from those presented earlier for the OpenMC-NekRS-MOOSE coupling. Therefore,
for the OpenMC and MOOSE heat conduction cases, we only point out the aspects
that differ from the earlier presentation. Differences will exist in three main
areas:

- Due to the use of the "tree" MultiApp hierarchy, different initial conditions
  are required because the applications execute in a different order
- Due to the use of the "tree" MultiApp hierarhcy, the wall heat flux and
  wall temperature exchanged between MOOSE and THM will need to go "up a level"
  to their common parent application, while the thermal-fluid application can now
  directly send fluid temperature to OpenMC
- THM requires a slightly different data transfer than NekRS due to its 1-D representation
  of the flow channels, which requires us to compute an average of the wall heat flux
  along the wall channel

#### OpenMC Input Files

The neutronics physics is solved over the entire domain with OpenMC.
The OpenMC wrapping used for the OpenMC-THM-MOOSE coupling is described in
the `openm_thm.i` input file. Relative to the OpenMC input files used for
the OpenMC-NekRS-MOOSE coupling shown previously, we now need to add variables
to hold the wall temperature, `thm_temp_wall`, that THM passes to the MOOSE heat conduction module,
and the wall heat flux, `flux`, that the MOOSE heat conduction module
passes to THM. Both of these are simply receiver variables that are written into
by the two sub-applications (MOOSE heat conduction and THM).

!listing /tutorials/gas_compact_multiphysics/openmc_thm.i
  block=AuxVariables

Next, when using the "tree" MultiApp hierarchy, OpenMC runs after the MOOSE
heat conduction module, but before THM. Therefore, initial conditions are required
for the fluid temperature (for OpenMC) as well as for the fluid wall temperature
(which will be sent to the MOOSE heat conduction module on the first time step).
Because OpenMC sends its heat source to the MOOSE heat conduction module at the
beginning of a time step, we also need to set an initial condition on the heat source.

!listing /tutorials/gas_compact_multiphysics/openmc_thm.i
  start=ICs
  end=Problem

The `[Problem]` block is identical to that shown earlier for the OpenMC-NekRS-MOOSE
coupling. Next, we define the two sub-applications and the transfers. It is important
to note that (at least at the time of writing this tutorial) THM and the MOOSE heat
conduction module cannot "talk" directly to one another, so there are transfers
of wall heat flux and wall temperature to and from OpenMC to one of the sub-applications,
giving four additional transfers in the main application's input file than we saw
for the "single-stack" hierarchy.

!listing /tutorials/gas_compact_multiphysics/openmc_thm.i
  start=MultiApps
  end=Postprocessors

!alert note
The advantage of the "tree" MultiApp hierarhcy is in the simpler solid input file
that we will see in [#solid2], which will not need to have a dummy part of the mesh
for the fluid region or any [NullKernels](https://mooseframework.inl.gov/source/kernels/NullKernel.html).

Finally, because we are coupling to THM, we need to average the wall heat flux
around the coolant channel into a number of layers to transfer from the 3-D MOOSE
heat conduction model to the 1-D fluid flow model. Heat flux is averaged using
a [LayeredSideAverage](https://mooseframework.inl.gov/source/userobject/LayeredSideAverage.html).

!listing /tutorials/gas_compact_multiphysics/openmc_thm.i
  block=UserObjects

All other aspects of the input file are the same as for the OpenMC-NekRS-MOOSE case.

#### Solid Input Files
  id=solid2

The solid heat conduction input file is the `solid_thm.i` input. This input file
is identical to the `solid_nek.i` input file for the OpenMC-NekRS-MOOSE case
except for:

- There are no MultiApps, and therefore no transfers, to a thermal-fluid code
- There are no initial conditions in the input file, since OpenMC sends the
  MOOSE heat conduction file all necessary initial conditions for wall temperature
  and heat source
- There is no need to explicitly turn off the material coverage check or add
  a [NullKernel](https://mooseframework.inl.gov/source/kernels/NullKernel.html)
  because there is not a dummy fluid block to receive fluid temperature from
  a sub-application

Because all of these differences are simply *omissions*, this concludes
the discussion of the solid input file.

#### Fluid Input Files

The fluid mass, momentum, and energy transport physics are solved using THM. The input
file for this portion of the physics is the `thm.i` input.
The THM input file is built using syntax specific to THM - we will only briefly
cover the syntax, and instead refer users to the THM manuals for more information.
First, we define a number of constants at the beginning of the file and apply
some global settings. We set the initial conditions for pressure, velocity, and
temperature and indicate the fluid [!ac](EOS) object using
[IdealGasFluidProperties](https://mooseframework.inl.gov/source/userobjects/IdealGasFluidProperties.html).

!listing /tutorials/gas_compact_multiphysics/thm.i
  end=AuxVariables

Next, we define the "components" in the domain. These components essentially consist
of the physics equations and boundary conditions solved by THM, but expressed
in THM-specific syntax. These components define single-phase flow in a pipe, an inlet
mass flowrate boundary condition, an outlet pressure condition, and heat transfer
to the pipe wall.

!listing /tutorials/gas_compact_multiphysics/thm.i
  block=Components

Associated with these components are a number of closures, defined as materials.
We set up the Churchill correlation for the friction factor and the Dittus-Boelter
correlation for the convective heat transfer coefficient. For the Dittus-Boelter
correlation, we use a corrected version of the closure (with the leading coefficient
changed from 0.023 to 0.021) based on the NekRS simulations described in
[Tutorial 2C](https://cardinal.cels.anl.gov/tutorials/cht3.html).
Additional materials are
created to represent dimensionless numbers and other auxiliary terms, such as the
wall temperature. As can be seen here, the [Material](https://mooseframework.inl.gov/syntax/Materials/index.html)
system is not always used to represent quantities traditioanlly thought of
as "material properties."

!listing /tutorials/gas_compact_multiphysics/thm.i
  block=Materials

THM computes the wall temperature to apply a boundary condition in the MOOSE
heat conduction module. To convert the `T_wall` material into an
auxiliary variable, we use the [ADMaterialRealAux](https://mooseframework.inl.gov/source/auxkernels/MaterialRealAux.html).

!listing /tutorials/gas_compact_multiphysics/thm.i
  start=AuxVariables
  end=Materials

Finally, we set the preconditioner, a [Transient](https://mooseframework.inl.gov/source/executioners/Transient.html)
executioner, and set an Exodus output. We will run THM to convergence based on a tight
steady state relative tolerance of $10^{-8}$.

!listing /tutorials/gas_compact_cht/thm.i
  start=Preconditioning

## Execution and Postprocessing

To run the coupled OpenMC-NekRS-MOOSE calculation, run the following:

```
mpiexec -np 500 cardinal-opt -i common_input.i openmc_nek.i
```

This will run with 500 [!ac](MPI) processes (you may run with other parallel
configurations as needed, but you will find that the NekRS simulation requires
HPC resources due to the large mesh). To run the coupled OpenMC-THM-MOOSE
calculation, run the following:

```
mpiexec -np 2 cardinal-opt -i common_input.i openmc_thm.i --n-threads=36
```

which will run with 2 [!ac](MPI) ranks with 36 threads each (again, these parallel
resource choices are only examples). Large computational resources are not required
for the THM-based model due to the much fewer degrees of freedom with the THM model.

When the two simulations have completed, you will have created a number of different
output files. For the OpenMC-NekRS-MOOSE calculation:

- `openmc_nek_out.e`, an Exodus file with the OpenMC solution
- `openmc_nek_out_bison0.e`, an Exodus file with the MOOSE heat conduction solution
- `openmc_nek_out_bison0_nek0.e`, an Exodus file with the NekRS solution
- `csv_nek/*`, CSV files with various postprocessors and userobjects

And for the OpenMC-THM-MOOSE calculation:

- `openmc_thm_out.e`, an Exodus file with the OpenMC solution
- `openmc_thm_out_bison0.e`, an Exodus file with the MOOSE heat conduction solution
- `openmc_thm_out_thm0.e`, an Exodus file with the THM solution
- `csv_thm/*`, CSV files with various postprocessors and userobjectsA

We now briefly present the coupled physics predictions.
[unit_cell_power] shows the fission power predicted by OpenMC with
thermal-fluid feedback from NekRS-MOOSE and THM-MOOSE. Also shown is the
difference between the two predictions (NekRS case minus the THM case).
For the six compacts with 50 axial cell layers, the OpenMC model contains a
total of 300 tallies; the maximum tally relative error of 1% does introduce some
slight asymmetries among the six compacts, which are easiest to discern in the "difference"
image in [unit_cell_power].

!media unit_cell_power.png
  id=unit_cell_power
  caption=Heat source predicted by OpenMC with thermal feedback from either NekRS-MOOSE or THM-MOOSE
  style=width:100%;margin-left:auto;margin-right:auto

The lack of reflectors results in a relatively high leakage percentage of about 18.8%.
For the unit cell, recall that the fluid temperature rise is only 82 K.
These two effects combine to give a power distribution that is very nearly
symmetric in the axial direction - while the lower solid temperatures near the
inlet do cause power to shift slightly downwards, the magnitude of the shift is
moderated by the effect of pushing the fission source closer to external
boundaries, where those neutrons would be more likely to exit the domain.

This tutorial is an extension of [Tutorial 2C](https://cardinal.cels.anl.gov/tutorials/cht3.html);
in that tutorial, we showed that [!ac](CHT) calculations with NekRS-MOOSE and THM-MOOSE
agree very well with one another in terms of fluid temperature, solid temperature, and fluid density.
Especially when considering that the neutronics feedback due to fluid temperature
and density are very small for gas-cooled systems, the excellent agreement in
OpenMC's fission distribution shown in [unit_cell_power] is as expected.
When cast in terms of a percent difference (as opposed to the absolute
difference shown in [unit_cell_power]), the NekRS-MOOSE-OpenMC and THM-MOOSE-OpenMC
cases agree to within 1%, which is within the range of the uncertainty in the fission tally itself.

[unit_cell_solid_temp] shows the solid temperature predicted by the
MOOSE heat conduction module, with physics feedback provided by either NekRS-OpenMC or THM-OpenMC.
Similar to the [!ac](CHT) case in [Tutorial 2C](https://cardinal.cels.anl.gov/tutorials/cht3.html)
, the solid temperature peaks
slightly downstream of the maximum OpenMC power due to the combined effects
of the power distribution and convective heat transfer at the wall.
[unit_cell_solid_temp] helps explain the trend in the power difference shown in
[unit_cell_power]. Near the inlet, NekRS predicts a lower temperature than THM,
while near the outlet NekRS predicts a higher temperature than THM. Due to the
negative temperature reactivity coefficient of the unit cell, this causes the
NekRS-based coupled model to predict a higher power than THM near the inlet,
but a lower power than THM near the outlet.

!media unit_cell_solid_temp.png
  id=unit_cell_solid_temp
  caption=Solid temperature predicted by the MOOSE heat conduction module with physics feedback from either OpenMC-NekRS or OpenMC-THM.
  style=width:100%;margin-left:auto;margin-right:auto

[unit_cell_solid_temp_midplane] shows the solid temperature predictions
on the axial midplane with physics feedback provided by either NekRS-OpenMC or THM-OpenMC.
Also shown is the difference between the two (NekRS-based case minus the THM-based case).
Similar to the [!ac](CHT) case in [Tutorial 2C](https://cardinal.cels.anl.gov/tutorials/cht3.html)
, the THM simulations are unable to capture the radial variation in heat flux along
the channel wall, causing THM to underpredict temperatures near the channel wall
close to the fuel compacts. However, an additional source of difference is now
also present - small radial asymmetries in OpenMC's fission distribution contribute
a small asymmetry in solid temperature on the order of 2 K. The tolerance on the
fission power uncertainty can simply be increased in order to push down this contribution.

!media unit_cell_solid_temp_midplane.png
  id=unit_cell_solid_temp_midplane
  caption=Solid temperature predicted by the MOOSE heat conduction module with physics feedback from either OpenMC-NekRS or OpenMC-THM on the axial mid-plane.
  style=width:100%;margin-left:auto;margin-right:auto

The OpenMC model receives a total of 400 solid temperatures (six compacts
plus two graphite cells, for each of the 50 axial layers), 50 fluid temperatures,
and 50 fluid densities from the thermal-fluid applications. [unit_cell_openmc_temp]
shows the solid temperature predicted by the NekRS-MOOSE-OpenMC high-resolution
simulations along with the actual cell temperature imposed in OpenMC for a
half-height slice of the unit cell. OpenMC sets a volume-average temperature for
each cell according to the mesh mirror element centroid mappings to OpenMC's cells.

!media unit_cell_openmc_temp.png
  id=unit_cell_openmc_temp
  caption=Solid temperature predicted by the MOOSE heat conduction module with physics feedback from OpenMC-NekRS and the solid temperature actually imposed in OpenMC for a half-height portion of the unit cell
  style=width:100%;margin-left:auto;margin-right:auto

[unit_cell_fluid_temp] shows the fluid temperature predicted by NekRS and THM,
with physics feedback provided by MOOSE-OpenMC. NekRS resolves the thermal
boundary layer, whereas the THM model uses the Dittus-Boelter correlation to
represent the temperature drop from the heated wall to the bulk. Therefore,
the fluid temperature shown in [unit_cell_fluid_temp] for THM is the area-averaged
fluid temperature. The wall temperature predicted by NekRS follows a very similar
distribution as the heat flux, with a profile that peaks slightly downstream of
the maximum power due to the combined effects of convective heat transfer and the fission power distribution.

!media unit_cell_fluid_temp.png
  id=unit_cell_fluid_temp
  caption=Fluid temperature predicted for the multiphysics simulations for NekRS-MOOSE-OpenMC and THM-MOOSE-OpenMC.
  style=width:100%;margin-left:auto;margin-right:auto

[unit_cell_coupled_axial_T] shows the radially-averaged temperatures for the two
thermal-fluid feedback options.
As already shown, the solid and fluid temperatures match very well between the two thermal-fluid feedback options. The fluid bulk temperature increases along the flow direction, with a faster rate of increase where the power density is highest.

!media unit_cell_coupled_axial_T.png
  id=unit_cell_coupled_axial_T
  caption=Radially-averaged temperatures for the NekRS-MOOSE-OpenMC and THM-MOOSE-OpenMC simulations.
  style=width:50%;margin-left:auto;margin-right:auto

