# Multiphysics for an SFR Pincell

In this tutorial, you will learn how to:

- Couple OpenMC, NekRS, and MOOSE for modeling an [!ac](SFR) pincell
- Use MOOSE's [reactor](https://mooseframework.inl.gov/modules/reactor/index.html) module
  to make meshes for common reactor geometries
- Use subcycling to efficiently allocate computational resources

To access this tutorial,

```
cd cardinal/tutorials/pincell_multiphysics
```

!alert! note title=Computing Needs
This tutorial does not require any special computing needs.
For testing purposes, you may choose to decrease the number of particles to solve faster.
!alert-end!

## Geometry and Computational Model

The geometry consists of a single pincell, cooled by sodium. The dimensions and assumed operating conditions
are summarized in [table1]. Note that these conditions are not necessarily representative
of full-power nuclear reactor conditions, since we elect laminar flow conditions in order
to reduce meshing requirements for the sake of a fast-running tutorial. By using
laminar flow, we will also use a much lower power density than seen in prototypical systems -
but for the sake of a tutorial, all the essential components of multiphysics are present
(coupling via power, temperature, and density). In addition, to keep the computational
requirements as low as possible for NekRS, the height is only 50 cm (so the
eignevalue predicted by OpenMC will also be very low, due to high axial leakage).

!table id=table1 caption=Geometric and operating specifications for a pincell
| Parameter | Value |
| :- | :- |
| Fuel pellet diameter, $d_f$ | 0.825 cm |
| Clad outer diameter, $d_c$ | 0.97 cm |
| Pin pitch, $p$ | 1.28 cm |
| Height | 50.0 cm |
| Reynolds number | 500.0 |
| Prandtl number | 0.00436 |
| Inlet temperature | 573 K |
| Outlet temperature | 708 K |
| Power | 250 W |

We will couple OpenMC, NekRS, and MOOSE heat conduction in a "single stack" hierarchy, with OpenMC
as the main application, MOOSE as the second-level application, and NekRS as the third-level
application. This structure is shown in [multiapp_pincell], and will allow us to sub-cycle
all three codes with respect to one another. Solid black lines indicate data transfers that occur
directly from application $A$ to application $B$, while the dashed black lines indicate data transfers
that first have to pass through an "intermediate" application.

!media multiapp_pincell.png
  id=multiapp_pincell
  caption="Single stack" multiapp hierarchy used in this tutorial.
  style=width:40%;margin-left:auto;margin-right:auto

Like with all Cardinal simulations, Picard iterations are achieved "in time." We have not expounded
greatly upon this notion in previous tutorials, so we dedicate some space here.
The overall Cardinal simulation has a notion of "time" and a time step index,
because all code applications will use a [Transient](https://mooseframework.inl.gov/source/executioners/Transient.html)
executioner. However,
usually only NekRS is solved with non-zero time derivatives. The notion of time-stepping is then used to
customize how frequently (i.e., in units of time steps) data is exchanged.
To help explain the strategy, represent the time step sizes in NekRS, MOOSE, and OpenMC as
$\Delta t_{nek}$, $\Delta t_{bison}=M\Delta t_{nek}$, and $\Delta t_{openmc}=NM\Delta t_{nek}$, respectively.
Selecting $N\neq1$ and/or $M\neq1$ is referred to as "sub-cycling." In other words,
NekRS runs $M$ times for each MOOSE solve, while MOOSE runs $N$ times for each OpenMC solve, effectively reducing the total number of MOOSE solves by a factor of $M$ and the total number of OpenMC solves by a factor of $NM$ compared to the naive approach to exchange data based on the smallest time step across the coupled codes.
We can then also reduce the total number of data *transfers* by only transferring
data when needed. Each Picard iteration consists of:

- Run an OpenMC $k$-eigenvalue calculation. Transfer the heat source $\dot{q}_s$ to MOOSE.
- Repeat $N$ times:

  - Run a steady-state MOOSE calculation. Transfer the heat flux $q^{''}$ to NekRS.
  - Run a transient NekRS calculation for $M$ time steps. Transfer the wall temperature $T_\text{wall}$ to MOOSE.

- Transfer the solid temperature $T_s$, the fluid temperature $T_f$, and the fluid density $\rho_f$ to OpenMC.

[subcycling2] shows the procedure for an example selection of $N=3$, meaning that the MOOSE-NekRS sub-solve occurs three times for every OpenMC solve.

!media subcycling.png
  id=subcycling2
  caption=Coupling procedure with subcycling for a calculation with OpenMC, MOOSE, and NekRS
  style=width:50%;margin-left:auto;margin-right:auto

In this tutorial, we will use different "time steps" for OpenMC, MOOSE, and NekRS,
and we invite you at the end of the tutorial to explore the impact on changing
these time step magnitudes to see how you can control the frequency of data transfers.

### OpenMC Model

OpenMC is used to solve for the neutron transport and power distribution.
The OpenMC model is created using OpenMC's Python API. We will build this geometry using [!ac](CSG)
and divided the pincell into a number of axial layers. On each layer, we will apply temperature
and density feedback from the coupled MOOSE-NekRS simulation.

First, we create a number of materials to represent
the fuel pellet and cladding. Because these materials will only receive temperature updates (i.e.
we will not change their density, because doing so would require us to move cell boundaries in
order to preserve mass), we can simply create one material that we will use on all axial layers.
Next, we create a number of materials to represent the sodium material in each
axial layer of the model. Because the sodium cells will change both their temperature *and*
density, we need to create a unique material for each axial layer.

Next, we divide the geometry into a number of axial layers, where on each
layer we set up cells to represent the pellet, cladding, and sodium. Each layer is then
described by lying between two $z$-planes. The boundary condition on the top and bottom
faces of the OpenMC model are set to vacuum.
Finally, we declare a number of
settings related to the initial fission source (uniform over the fissionale regions)
and how temperature feedback is applied, and then export all files to XML.

!listing /tutorials/pincell_multiphysics/pincell.py

You can create these XML files by running

```
python pincell.py
```

### Heat Conduction Model

The MOOSE heat conduction module is used to solve for [energy conservation in the solid](theory/heat_eqn.md).
The solid mesh is shown in [solid_mesh]. This mesh is generated using MOOSE's
[reactor module](https://mooseframework.inl.gov/modules/reactor/index.html), which can be used to make
sophisticated meshes of typical reactor geometries such as pin lattices, ducts, and reactor vessels.
In this example, we use this module to set up just a single pincell.

!media pincell_solid.png
  id=solid_mesh
  caption=Solid mesh
  style=width:60%;margin-left:auto;margin-right:auto

This mesh is described in the `solid.i` file,

!listing tutorials/pincell_multiphysics/solid.i
  block=Mesh

You can generate this mesh by running

```
cardinal-opt -i solid.i --mesh-only
```

which will generate the mesh as the `solid_in.e` file. You will note in [solid_mesh] that this
mesh actually also includes mesh for the fluid region. In [multiapp_pincell], we need a mesh region
to "receive" the fluid temperature and density from NekRS, because NekRS cannot directly communicate
with OpenMC by skipping the second-level app (MOOSE). Therefore, no solve will occur on this fluid mesh,
it simply exists to receive the fluid solution from NekRS before being passed "upwards" to OpenMC.
By having this dummy "receiving-only" mesh in the solid application, we will require a few
extra steps when we set up the MOOSE heat conduction model in [#solid_model].

### NekRS Model

NekRS is used to solve the [incompressible Navier-Stokes equations](theory/ins.md) in
[non-dimensional form](theory/nondimensional_ns.md).
The NekRS input files needed to solve the incompressible Navier-Stokes equations are:

- `fluid.re2`: NekRS mesh (generated by `exo2nek` starting from MOOSE-generated meshes, to be discussed shortly)
- `fluid.par`: High-level settings for the solver, boundary condition mappings to sidesets, and the equations to solve
- `fluid.udf`: User-defined C++ functions for on-line postprocessing and model setup
- `fluid.oudf`: User-defined [!ac](OCCA) kernels for boundary conditions and source terms

A detailed description of all of the available parameters, settings, and use
cases for these input files is available on the
[NekRS documentation website](https://nekrsdoc.readthedocs.io/en/latest/index.html).
Because the purpose of this analysis is to demonstrate Cardinal's capabilities, only the aspects
of NekRS required to understand the present case will be covered.

We first create the mesh using MOOSE's [reactor module](https://mooseframework.inl.gov/modules/reactor/index.html).
The syntax used to build a HEX27 fluid mesh is shown below. A major difference from the dummy
fluid receiving portion of the solid mesh is that we now set up some
boundary layers on the pincell surfaces, by providing `ring_radii` (and other parameters) as vectors.

!listing tutorials/pincell_multiphysics/fluid.i
  block=Mesh

However, NekRS requires a HEX20 mesh format. Therefore, we use a
[NekMeshGenerator](/meshgenerators/NekMeshGenerator.md) to convert from HEX27 to HEX20 format,
while also moving the higher-order side nodes on the pincell surface to match the curvilinear elements.
The syntax used to convert from the HEX27 fluid mesh to a HEX20 fluid mesh, while preserving
the pincell surface, is shown below.

!listing tutorials/pincell_multiphysics/convert.i

To generate the meshes, we run:

```
cardinal-opt -i fluid.i --mesh-only
cardinal-opt -i convert.i --mesh-only
mv convert_in.e convert.exo
```

and then use the `exo2nek` [utility](https://cardinal.cels.anl.gov/nek_tools.html)
to convert from the exodus file format (`convert.exo`) into the custom `.re2` format
required for NekRS. A depiction of the outputs of the two stages of the mesh generator
process are shown in [fluid_convert]. Boundary 2 is the inlet, boundary 3 is the outlet,
and boundary 1 is the pincell surface. Boundaries 4 through 7 are the lateral faces of
the pincell.

!media fluid_convert.png
  id=fluid_convert
  caption=Outputs of the mesh generators used for the NekRS flow domain
  style=width:80%;margin-left:auto;margin-right:auto

Next, the `.par` file contains problem setup information. This input solves
for pressure, velocity, and temperature.
In the nondimensional formulation,
the "viscosity" becomes $1/Re$, where $Re$ is the Reynolds number, while the
"thermal conductivity" becomes $1/Pe$, where $Pe$ is the Peclet number. These nondimensional
numbers are used to set various diffusion coefficients in the governing equations
with syntax like `-500.0`, which is equivalent in NekRS syntax to $\frac{1}{500.0}$.
On the four lateral faces of the pincell, we set symmetry conditions for velocity.

!listing /tutorials/pincell_multiphysics/fluid.par

Next, the `.udf` file is used to set up initial conditions for velocity,
pressure, and temperature. We set uniform axial velocity initial conditions,
and temperature to a linear variation from 0 at the inlet to 1 at the outlet.

!listing /tutorials/pincell_multiphysics/fluid.udf language=cpp

In the `.oudf` file, we define boundary conditions. On the flux boundary, we will
be sending a heat flux from MOOSE into NekRS, so we set the flux equal to the scratch
space array, `bc->wrk[bc->idM]`.

!listing /tutorials/pincell_multiphysics/fluid.oudf language=cpp

## Multiphysics Coupling

In this section, OpenMC, NekRS, and MOOSE are coupled for multiphysics modeling of an [!ac](SFR) pincell.
This section describes all input files.

### OpenMC Input Files

The neutron transport is solved using OpenMC. The input file for this portion of the physics is `openmc.i`. We begin by defining a number of constants and setting up the mesh mirror. For simplicity, we just use the same mesh as used in the solid application, shown in [solid_mesh].

!listing tutorials/pincell_multiphysics/openmc.i
  end=AuxVariables

Next, we define a number of auxiliary variables to be used for diagnostic purposes. With the exception of the [FluidDensityAux](https://mooseframework.inl.gov/source/auxkernels/FluidDensityAux.html), none of the following variables are necessary for coupling, but they will allow us to visualize how data is mapped from OpenMC to the mesh mirror. The [FluidDensityAux](https://mooseframework.inl.gov/source/auxkernels/FluidDensityAux.html)
auxiliary kernel on the other hand is used to compute the fluid density, given the temperature
variable `temp` (into which we will write the MOOSE and NekRS temperatures, into different regions of space). Note that we will not send fluid density from NekRS to OpenMC, because the NekRS model uses an incompressible Navier-Stokes model. But to a decent approximation, the fluid density can be approximated solely as a function of temperature using the
[SodiumSaturationFluidProperties](https://mooseframework.inl.gov/source/userobjects/SodiumSaturationFluidProperties.html)
(so named because these properties represent sodium properties at
saturation temperature).

!listing tutorials/pincell_multiphysics/openmc.i
  start=AuxVariables
  end=ICs

Next, the `Problem` block defines all the parameters related to coupling
OpenMC to MOOSE. We will send temperature to OpenMC from blocks 2 and 3
(which represent the solid regions) and we will send temperature and density
to OpenMC from block 1 (which represents the fluid region). We will have
Cardinal automatically set up cell tallies.

For this problem, the temperature that gets mapped into OpenMC is sourced
from two different applications, which we can customize using the
`temperature_variables` and `temperature_blocks` parameters. Later
in the `Transfers` block, all data transfers from the sub-applications will write
into either `solid_temp` or `nek_temp`.

Finally, we provide some additional specifications for how to run OpenMC.
We set the number of batches and various triggers that will automatically
terminate the OpenMC solution when reaching less than 2% maximum relative
uncertainty in the fission power taly and 7.5E-4 standard deviation in $k$.

!listing tutorials/pincell_multiphysics/openmc.i
  block=Problem

Next, we set up some initial conditions for the various fields used for coupling.

!listing tutorials/pincell_multiphysics/openmc.i
  start=ICs
  end=Problem

Next, we create a MOOSE heat conduction sub-application, and set up transfers of data between OpenMC and MOOSE. These transfers will send solid temperature and fluid temperature from MOOSE up to OpenMC, and a power distribution to MOOSE.

!listing tutorials/pincell_multiphysics/openmc.i
  start=MultiApps
  end=Outputs

Finally, we will use a [Transient](https://mooseframework.inl.gov/source/executioners/Transient.html) executioner and add a number of postprocessors for diagnostic purposes.

!listing tutorials/pincell_multiphysics/openmc.i
  start=Outputs

### Solid Input Files
  id=solid_model

The conservation of solid energy is solved using the MOOSE heat conduction module. The input
file for this portion of the physics is `bison.i`. We begin by defining a
number of constants and setting the mesh.

!listing tutorials/pincell_multiphysics/bison.i
  end=Problem

Because some blocks in the solid mesh aren't actually used in the solid solve
(i.e. the fluid block we will use to receive fluid temperature and density from
NekRS), we need to explicitly tell MOOSE to not throw errors related
to checking for material properties on every mesh block.

!listing tutorials/pincell_multiphysics/bison.i
  block=Problem

Next, we set up a nonlinear variable `T` to represent solid temperature
and create kernels representing heat conduction with a volumetric heating
in the pellet region. In the fluid region, we need to use a
[NullKernel](https://mooseframework.inl.gov/source/kernels/NullKernel.html)
to indicate that no actual solve happens on this block. On the
cladding surface, we will impose a Dirichlet boundary condition given
the NekRS fluid temperature. Finally, we set up material properties for
the solid blocks using [HeatConductionMaterial](https://mooseframework.inl.gov/source/materials/HeatConductionMaterial.html).

!listing tutorials/pincell_multiphysics/bison.i
  start=Variables
  end=AuxVariables

Next, we declare auxiliary variables to be used for:

- coupling to NekRS (to receive the NekRS temperature, `nek_temp`)
- computing the pin surface heat flux (to send heat flux `flux` to NekRS)
- `power` to represent the volumetric heating received from OpenMC

On each MOOSE-NekRS substep, we will run MOOSE first. For the very first
time step, this means we should set an initial condition for the NekRS
fluid temperature, which we simply set to a linear function of height.
Finally, we create a [DiffusionFluxAux](https://mooseframework.inl.gov/source/auxkernels/DiffusionFluxAux.html)
to compute the heat flux on the pin surface.

!listing tutorials/pincell_multiphysics/bison.i
  start=AuxVariables
  end=MultiApps

Next, we create a NekRS sub-application and set up the data transfers between
MOOSE and NekRS. There are four transfers - three are related to sending the temperature and heat flux between MOOSE and NekRS, and the fourth is related to
a performance optimization in the NekRS wrapping that will only interpolate data to/from NekRS at the synchronization points with the application "driving" NekRS (in this case, MOOSE heat conduction).

!listing tutorials/pincell_multiphysics/bison.i
  start=MultiApps
  end=Postprocessors

Next, we set up a number of postprocessors, both for normalizing the
total power sent from MOOSE to NekRS, as well as a few diagnostic terms.

!listing tutorials/pincell_multiphysics/bison.i
  block=Postprocessors

Finally, we set up the MOOSE heat conduction solver to use a
[Transient](https://mooseframework.inl.gov/source/executioners/Transient.html)
exeuctioner and specify the output format. Important to note here is that a user-provided choice of `M` will determine how many NekRS time steps are run for *each* MOOSE time step.

!listing tutorials/pincell_multiphysics/bison.i
  start=Executioner

### Fluid Input Files

The fluid mass, momentum, and energy transport physics are solved using NekRS. The input file
for this portion of the physics is `nek.i`. We begin by defining a number of constants and by setting
up the [NekRSMesh](mesh/NekRSMesh.md) mesh mirror. Because we are coupling NekRS via boundary heat
transfer to MOOSE, but via volumetric temperature and densities to OpenMC, we need to use a combined
boundary *and* volumetric mesh mirror, so both `boundary` and `volume = true` are provided. Because
this problem is set up in non-dimensional form, we also need to rescale the mesh to match the units
expected by our solid and OpenMC input files.

!listing tutorials/pincell_multiphysics/nek.i
  end=Problem

The bulk of the NekRS wrapping is specified with
[NekRSProblem](https://cardinal.cels.anl.gov/source/problems/NekRSProblem.html).
The NekRS input files are in non-dimensional form, whereas all other coupled applications
use dimensional units. The various `*_ref` and `*_0` parameters define the characteristic
scales that were used to non-dimensionalize the NekRS input. In order to simplify the input
file, we know a priori that OpenMC will not be sending a heat source *to NekRS*, so
we set `has_heat_source = false` so that we don't need to add a dummy heat
source kernel to the `fluid.oudf` file. If we had volumetric heating in the *fluid*,
then we would instead send a heating term into NekRS, but this is neglected for this example.

!listing /tutorials/pincell_multiphysics/nek.i
  block=Problem

Then, we simply set up a [Transient](https://mooseframework.inl.gov/source/executioners/Transient.html)
executioner with the [NekTimeStepper](https://cardinal.cels.anl.gov/source/timesteppers/NekTimeStepper.html).
For postprocessing purposes, we also create two postprocessors to evaluate the average outlet
temperature and the maximum fluid temperature. Finally, in order to not overly saturate the screen
output, we will only write the Exodus output files and print to the console every 10 Nek time steps.

!listing tutorials/pincell_multiphysics/nek.i
  start=Executioner

## Execution

To run the input files,

```
mpiexec -np 4 cardinal-opt -i openmc.i
```

This will produce a number of output files,

- `openmc_out.e`, OpenMC simulation results, mapped to a mesh mirror
- `openmc_out_bison0.e`, MOOSE heat conduction simulation results
- `openmc_out_bison0_nek0.e`, NekRS simulation results, mapped to a mesh mirror
- `fluid0.f*`, NekRS output files (which you can visualize in Paraview/other software by using the `visnek` [NekRS script](https://cardinal.cels.anl.gov/nek_tools.html)

[pincell_temp] shows the temperature (a) imposed in OpenMC (the output
of the [CellTemperatureAux](https://cardinal.cels.anl.gov/source/auxkernels/CellTemperatureAux.html))
auxiliary kernel; (b) the NekRS fluid temperature; and (c) the MOOSE
solid temperature.

!media pincell_temp.png
  id=pincell_temp
  caption=Temperatures predicted by NekRS and BISON (middle, right), and what is imposed in OpenMC (left). All images are shown on the same color scale.
  style=width:100%;margin-left:auto;margin-right:auto

The OpenMC power distribution is shown in [pincell_power]. The small
variations of sodium density with temperature cause the power distribution
to be fairly symmetric in the axial direction.

!media pincell_power.png
  id=pincell_power
  caption=OpenMC predicted power distribution
  style=width:40%;margin-left:auto;margin-right:auto

Finally, [pincell_cross] shows the fluid and solid temperatures (a) with both phases shown and (b) with just the fluid region highlighted. The very lower power in this demonstration results in fairly small temperature gradients in the fuel, but what is important to note is the coupled solution captures typical conjugate heat transfer temperature distributions in rectangular pincells.
You can always extend this tutorial to turbulent conditions and increase the power to display
temperature distributions more characteristic of real nuclear systems.

!media pincell_cross.png
  id=pincell_cross
  caption=Temperatures predicted by NekRS and BISON on the axial midplane.
  style=width:80%;margin-left:auto;margin-right:auto

In this tutorial, OpenMC, MOOSE, and NekRS each used their own time step.
You may want to try re-running this tutorial using different time step sizes
in the OpenMC and MOOSE heat conduction input files, to explore how to sub-cycle these codes together.
