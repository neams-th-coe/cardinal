# Conjugate Heat Transfer for Turbulent Channel Flow
  id=tutorial1c

In this tutorial, you will learn how to:

- Couple NekRS to MOOSE for [!ac](CHT) for turbulent flow in a [!ac](TRISO) fuel unit cell
- Couple NekRS to MOOSE for solving NekRS in non-dimensional form
- Extract an initial condition from a NekRS restart file
- Swap out NekRS for a different thermal-fluid MOOSE application, the [Thermal Hydraulics Module (THM)](https://mooseframework.inl.gov/modules/thermal_hydraulics/index.html)
- Compute heat transfer coefficients with NekRS

To access this tutorial,

```
cd cardinal/tutorials/gas_compact_cht
```

This tutorial also requires you to download some mesh files from Box.
Please download the files from the `gas_compact_cht` folder
[here](https://anl.app.box.com/s/irryqrx97n5vi4jmct1e3roqgmhzic89/folder/141527707499)
and place these files within the same directory structure in
`tutorials/gas_compact_cht`.

This tutorial was developed with support from the NEAMS Thermal Fluids
Center of Excellence and the NRIC [!ac](VTB). You can find additional context on
this model in our journal article [!cite](novak2022_cardinal).

!alert! note title=Computing Needs
This tutorial requires [!ac](HPC) resources to run the NekRS cases. Please still read this tutorial
if you do not have resources, because you will be able to run the THM cases
and much of the Cardinal usage is agnostic of the particular thermal code being used.
!alert-end!

## Geometry and Computational Model

The geometry consists of a unit cell of a [!ac](TRISO)-fueled
gas reactor compact [!cite](sterbentz).
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

### Heat Conduction Model

The MOOSE heat conduction module is used to solve for [energy conservation in the solid](theory/heat_eqn.md),
with the time derivative neglected in order to more quickly approach steady state.
The solid mesh is shown in [solid_mesh].
The [!ac](TRISO) particles are homogenized into
the compact regions - all material properties in the heterogeneous regions
are taken as volume averages of the various constituent materials.

!media compact_solid_mesh.png
  id=solid_mesh
  caption=Mesh for the solid heat conduction model
  style=width:60%;margin-left:auto;margin-right:auto

The volumetric power density $\dot{q}_s$ is set to a sinusoidal function of $z$,

\begin{equation}
\dot{q}_s=q_0\sin{\left(\frac{\pi z}{H}\right)}
\end{equation}

where $q_0$ is a coefficient used to ensure that the total power produced is 38 kW
and $H$ is the domain height. The power is uniform
in the radial direction in each compact.
On the coolant channel surface, a Dirichlet temperature is provided by NekRS/THM.
All other boundaries are insulated. We will run the solid model first, so we must specify
an initial condition for the wall temperature, which we simply set to a linear variation
between the inlet and the outlet based on the nominal temperature rise.

### NekRS Model

NekRS solves the [incompressible k-tau RANS equations](theory/ktau.md).
To relate $\mu_T$ to $k_T$, $Pr_T=0.91$ is selected. The resolution of the mesh near all no-slip boundaries
ensures that $y^+<1$ such that the NekRS model is a wall-resolved model.

Here, the NekRS case will be set up in [non-dimensional form](nondimensional_ns.md).
This is a convenient technique for fluid solvers, because the solution will be correct
for *any* flow at a given Reynolds/Peclet/etc. number in this geometry. A common user
operation is also to "ramp" up a CFD solve for stability purposes. Solving in non-dimensional
form means that you do not need to also "ramp" absolute values for velocity/pressure/temperature/etc. - switching to a different set of non-dimensional numbers is as simple
as just modifying the thermophysical "properties" of the fluid (e.g. viscosity, conductivity, etc.).

When the Navier-Stokes equations
are written in [non-dimensional form](nondimensional_ns.md), the "density" in the `[VELOCITY]` block becomes unity because

\begin{equation}
\label{eq:nondim_p}
\rho^\dagger\equiv\frac{\rho_f}{\rho_0}=1
\end{equation}

The "viscosity" becomes the coefficient on the
viscous stress term ($1/Re$, where $Re$ is the Reynolds number).
In NekRS, specifying `diffusivity = -50.0` is equivalent to specifying
`diffusivity = 0.02` (i.e. $1/50.0$), or a Reynolds number of 50.0.

In non-dimensional form, the `rhoCp` term in the `[TEMPERATURE]` block becomes unity because

\begin{equation}
\label{eq:nek1}
\rho^\dagger C_{p,f}^\dagger\equiv\frac{\rho_fC_{p,f}}{\rho_0C_{p,0}}=1
\end{equation}

The "conductivity" indicates the coefficient on the diffusion kernel, which in non-dimensional
form is equal to $1/Pe$, where $Pe$ is the Peclet number. In NekRS, specifying `conductivity = -35` is equivalent
to specifying `conductivity = 0.02857` (i.e. $1/35.0$), or a Peclet number of 35.

The inlet mass flowrate is 0.0905 kg/s; with the channel diameter of 1.6 cm and material
properties of helium, this results in a Reynolds number of 223214 and a Prandtl number
of 0.655. This highly-turbulent flow results in extremely thin momentum and thermal boundary
layers on the no-slip surfaces forming the periphery of the coolant channel. In order to
resolve the near-wall behavior, an extremely fine mesh is
required in the NekRS simulation. To accelerate the overall coupled [!ac](CHT) case
that is of interest in this tutorial, the NekRS model is split into a series of calculations:

1. We first run a partial-height, periodic flow-only case
   to obtain converged $P$, $\vec{u}$, and $\mu_T$ distributions.
2. Then, we extrapolate the $\vec{u}$ and $\mu_T$ to the full-height case.
3. Finally, we use the converged, full-height $\vec{u}$ and $\mu_T$ distributions
   to transport a temperature passive scalar in a [!ac](CHT) calculation with MOOSE.

As a rough estimate, solving the coupled mass-momentum equations requires about an
order of magnitude more compute time than a passive scalar equation in NekRS.
Therefore, by "freezing" the $\vec{u}$ and $\mu_T$ solutions,
we can dramatically reduce the total cost of the coupled [!ac](CHT) calculation.

The periodic flow model has a height of $\frac{H}{10}$ and takes about 10,000 core hours
to reach steady state. The extrapolation from $\frac{H}{10}$ to $H$ then is a simple postprocessing
operation. Because steps 1 and 2 are done exclusively using NekRS, and the periodic flow solve
takes a very long time to run (from the perspective of a tutorial, at least), we omit steps 1
and 2 from this tutorial. Instead, we begin straight away from a full-height periodic restart
file with the $k$-$\tau$ [!ac](RANS) model. This restart file is the
`periodic_flow_solve.f00001` file. If you open the restart file, you will see the following
converged NekRS predictions of velocity, $k$, and $\tau$.
[nek_line_plots] shows the axial velocity, $k$, and $\tau$ on a line plot through the center of the channel.

!media nek_uc_results.png
  id=nek_restart
  caption=Converged NekRS predictions of velocity magnitude (left), $k$ (middle), and $\tau$ (right) for the periodic case (all in non-dimensional form)
  style=width:80%;margin-left:auto;margin-right:auto

!media nek_line_plots.png
  id=nek_line_plots
  caption=Converged NekRS predictions of velocity magnitude (left), $k$ (middle), and $\tau$ (right) for the periodic case along the channel mide-plane (all in non-dimensional form)
  style=width:80%;margin-left:auto;margin-right:auto

For the conjugate heat transfer case, we will load this restart file, compute $k_T$ from the
loaded solutions for $k$ and $\tau$, and then transport temperature with coupling to MOOSE
heat conduction. The NekRS files are:

- `ranstube.re2`: NekRS mesh
- `ranstube.par`: High-level settings for the solver, boundary condition mappings to sidesets, and the equations to solve
- `ranstube.udf`: User-defined C++ functions for on-line postprocessing and model setup
- `ranstube.oudf`: User-defined [!ac](OCCA) kernels for boundary conditions and source terms

The NekRS mesh is shown in [nek_mesh]. Boundary 1 is the inlet, boundary 2 is the outlet, and boundary
3 is the wall. The same mesh was used for the periodic flow solve, except with
a shorter height.

!media nek_mesh_uc.png
  id=nek_mesh
  caption=Mesh for the NekRS [!ac](RANS) model
  style=width:60%;margin-left:auto;margin-right:auto

Next, the `.par` file contains problem setup information.
This input sets up a nondimensional passive scalar solution, loading $P$, $\vec{u}$,
$k$, and $\tau$ from a restart file. In order to "freeze," or turn off the $P$, $\vec{u}$,
$k$, and $\tau$ solves, we set `solver = none` in the `[VELOCITY]`, `[SCALAR01]` ($k$ passive scalar),
and `[SCALAR02]` ($\tau$ passive scalar) blocks.
The only equation that NekRS will solve is for temperature.

!listing /tutorials/gas_compact_cht/ranstube.par

Next, the `.udf` file is used to setup initial conditions and define how
$k_T$ should be computed based on $Pr_T$ and the restart values of $k$ and $\tau$.
In `turbulent_props`, a user-defined function, we use $k_f$ from the input file
in combination with the $Pr_T$ and $\mu_T$ (read from the restart file later in
the `.udf` file) to adjust the total diffusion coefficient on temperature to
$k_f+k_T$ according to the [definition of turbulent Prandtl number](theory/ktau.md). This adjustment must happen on device, in a new GPU kernel we name
`scalarScaledAddKernel`. This kernel will be defined in the `.oudf` file; we
instruct the JIT compilation to compile this new kernel by calling
`udfBuildKernel`.

Then, in `UDF_Setup` we set an initial condition for fluid
temperature (the first scalar in the `nrs->cds->S` array that holds all the
scalars). In this function, we also store the value of $\mu_T$ computed in the
restart file.

!listing /tutorials/gas_compact_cht/ranstube.udf language=cpp

In the `.oudf` file, we define boundary conditions for temperature and also
the form of the `scalarScaledAdd` kernel that we use to compute $k_T$.
The inlet boundary is set to a temperature of 0 (a dimensional temperature of
$T_{ref}$), while the fluid-solid interface will receive a heat flux from MOOSE.

!listing /tutorials/gas_compact_cht/ranstube.oudf language=cpp

### THM Model

THM solves the [1-D area averages of the Navier-Stokes equations](theory/thm.md).
The Churchill correlation is used for $f$ and the Dittus-Boelter correlation is used for $H_w$ [!cite](relap7).

The THM mesh contains 150 elements; the mesh is constucted automatically
within THM. The heat flux imposed in the THM elements is obtained
by area averaging the heat flux from the heat conduction model in 150 layers along
the fluid-solid interface. For the reverse transfer, the wall temperature sent to MOOSE
heat conduction is set to a uniform value along the fluid-solid interface according to
a nearest-node mapping to the THM elements.

Because the thermal-fluids tools run after the MOOSE heat conduction application, initial
conditions are only required for pressure, fluid temperature, and velocity,
which are set to uniform distributions.

## CHT Coupling
  id=cht

In this section, MOOSE, NekRS, and THM are coupled for [!ac](CHT).
Two separate simulations are performed here:

- Coupling of NekRS with MOOSE
- Coupling of THM with MOOSE

By individually describing the two setups, you will understand that NekRS is really
as interchangeable as any other MOOSE application for coupling.
In addition, we will describe how to generate a heat transfer coefficient from the
NekRS-MOOSE coupling to input into the THM-MOOSE coupling to demonstrate an important
multiscale use case of NekRS.

### NekRS-MOOSE

In this section, we describe the coupling of MOOSE and NekRS.

#### Solid Input Files

The solid phase is solved with the MOOSE heat conduction module, and is described in
the `solid_nek.i` input. We define a number of constants at the beginning of
the file and set up the mesh.

!listing /tutorials/gas_compact_cht/solid_nek.i
  end=Variables

Next, we define a temperature variable `T`, and specify the governing equations and
boundary conditions we will apply.

!listing /tutorials/gas_compact_cht/solid_nek.i
  start=Variables
  end=Functions

The MOOSE heat conduction module will receive a wall temperature from NekRS in
the form of an [AuxVariable](https://mooseframework.inl.gov/syntax/AuxVariables/index.html),
so we define a receiver variable for the temperature, as `fluid_temp`. The MOOSE
heat conduction module will also send heat flux to NekRS, which we compute as
another [AuxVariable](https://mooseframework.inl.gov/syntax/AuxVariables/index.html)
named `flux`, which we compute with a
[DiffusionFluxAux](https://mooseframework.inl.gov/source/auxkernels/DiffusionFluxAux.html)
auxiliary kernel. Finally, we define another auxiliary variable for the imposed power,
which we will not receive from a coupled application, but instead set within the solid
input file.

!listing /tutorials/gas_compact_cht/solid_nek.i
  start=AuxVariables
  end=MultiApps

Next, we use functions to define the thermal conductivities. The material properties
for the [!ac](TRISO) compacts are taken as volume averages of the various
constituent materials. We also use functions to set initial conditions for power
(a sinusoidal function) and the initial fluid temperature (a linear variation
from inlet to outlet).

!listing /tutorials/gas_compact_cht/solid_nek.i
  start=Functions
  end=Postprocessors

We define a number of postprocessors for querying the solution as well as for
normalizing the heat flux.

!listing /tutorials/gas_compact_cht/solid_nek.i
  block=Postprocessors

For visualization purposes only, we add [LayeredAverages](https://mooseframework.inl.gov/source/userobject/LayeredAverage.html)
to average the fuel and block temperatures in layers in the $z$ direction. We
also add a [LayeredSideAverage](https://mooseframework.inl.gov/source/userobject/LayeredSideAverage.html)
to average the heat flux along a boundary, again in layers in the $z$ direction.
We then output the results of these userobjects to CSV using
[SpatialUserObjectVectorPostprocessors](https://mooseframework.inl.gov/source/vectorpostprocessors/SpatialUserObjectVectorPostprocessor.html)
and by setting `csv = true` in the output.

!listing /tutorials/gas_compact_cht/solid_nek.i
  start=UserObjects

Finally, we add a [TransientMultiApp](https://mooseframework.inl.gov/source/multiapps/TransientMultiApp.html)
that will run a MOOSE-wrapped NekRS simulation. Then, we add four different
transfers to/from NekRS:

- [MultiAppGeneralFieldNearestNodeTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppGeneralFieldNearestNodeTransfer.html)
  to send the heat flux from MOOSE to NekRS
- [MultiAppGeneralFieldNearestNodeTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppGeneralFieldNearestNodeTransfer.html)
  to send temperature from NekRS to MOOSE
- [MultiAppPostprocessorTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppPostprocessorTransfer.html)
  to normalize the heat flux sent to NekRS
- [MultiAppPostprocessorTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppPostprocessorTransfer.html)
  to send a dummy postprocessor named `synchronization_to_nek` that
  simply allows the NekRS sub-application to know when "new" coupling data is
  available from MOOSE, and to only do transfers to/from GPU when new data is
  available; please consult the [NekRSProblem](https://cardinal.cels.anl.gov/source/problems/NekRSProblem.html)
  documentation for further information.

!listing /tutorials/gas_compact_cht/solid_nek.i
  start=MultiApps
  end=Executioner

Finally, although our heat conduction model does not have any time derivatives
(in order to more quickly reach steady state), NekRS must be run
as a transient. In order to control the interval on which NekRS couples to MOOSE, we
use a MOOSE time step that is `N` times bigger than the NekRS time step, taking care
to account for the fact that the time step in the `ranstube.par` file is actually
a *nondimensional* time step. By setting `subcycling = true` for the
[TransientMultiApp](https://mooseframework.inl.gov/source/multiapps/TransientMultiApp.html),
MOOSE is only run every `N` time steps (the subcycling setting), *and* that the NekRS
solution is only copied to/from GPU every `N` time steps (the minimize transfers setting).

Finally, we will run the MOOSE heat conduction model
until the relative change in the solution (across the entire domain) differs by
less than $10^{-1}$ from the previous time step. While this may seem like a large
tolerance, because the difference is taken over the entire domain, you will see changes
only on the order of the fourth or fifth decimal place in temperatures.

!listing /tutorials/gas_compact_cht/solid_nek.i
  block=Executioner

#### Fluid Input Files
  id=nek

The Nek wrapping is described in the `nek.i` input file.
We first define
a few file-local variables, and then build a mesh mirror with a
[NekRSMesh](https://cardinal.cels.anl.gov/source/mesh/NekRSMesh.html). By setting
`boundary = '3'`, we indicate that boundary 3 will be coupled via [!ac](CHT).
In this example, we don't have any volume-based coupling, but we set
`volume = true` to visualize the NekRS solution in Exodus format
(as a first-order interpolation of the 7-th order polynomial
solution).

When solving in MOOSE, all the coupled applications must either be in non-dimensional form,
or all in dimensional form. In this tutorial, we have set up the MOOSE input file
in dimensional form, but the NekRS case files were set up in non-dimensional form.
In order for MOOSE's transfers
to correctly find the closest nodes in the solid mesh to corresponding locations in NekRS, the entire mesh "mirror" through which data transfers occur must be
scaled by a factor of $L_{ref}$ to return to dimensional units (because the coupled MOOSE
application is in dimensional units). This scaling is specified by the
`scaling` parameter. In other words, the `scaling` indicates by what factor
we should internally scale spatial lengths by to correctly send data
between NekRS and MOOSE.

!listing /tutorials/gas_compact_cht/nek.i
  end=Problem

Next, we define additional parameters to describe how NekRS interacts with MOOSE
with the [NekRSProblem](https://cardinal.cels.anl.gov/source/problems/NekRSProblem.html).

To allow conversion between a non-dimensional NekRS solve and a dimensional MOOSE coupled
heat conduction application, the characteristic scales used to establish the non-dimensional
problem are provided. Definitions for these non-dimensional scales are
available [here](theory/nondimensional_ns.md).

!alert warning
These characteristic scales are used by Cardinal to dimensionalize the NekRS solution
into the units that the coupled MOOSE application expects. *You* still need to properly
non-dimensionalize the NekRS input files (to be discussed later).

We also indicate that we are going to restrict the data copies to/from
GPU to only occur on the time steps that NekRS is coupled to MOOSE.

!listing /tutorials/gas_compact_cht/nek.i
  block=Problem

For time stepping, we will allow NekRS to control its own time stepping
by using the [NekTimeStepper](https://cardinal.cels.anl.gov/source/timesteppers/NekTimeStepper.html).
We will output both Exodus and CSV format data as well.

!listing /tutorials/gas_compact_cht/nek.i
  start=Executioner
  end=Postprocessors

Then, we define a few postprocessors to use for querying the solution. While
we hide these from the console output above, they will still be output to CSV.

!listing /tutorials/gas_compact_cht/nek.i
  block=Postprocessors

Finally, we add a few userobjects to compute the necessary terms in a heat transfer
coefficient in a series of equally-spaced axial layers along the flow direction as

\begin{equation}
\label{eq:htc2}
q_i^{''}=h_i\left(T_{\text{wall},i}-T_{\text{bulk},i}\right)
\end{equation}

where $i$ is the layer index, $q_i^{''}$ is the average wall heat flux in layer $i$, $h$ is the heat transfer coefficient in layer $i$, $T_{\text{wall},i}$ is the average wall temperature in layer $i$, and $T_{\text{bulk},i}$ is the volume-averaged temperature in layer $i$.
All terms in [eq:htc2] can be computed using objects in the MOOSE and NekRS-wrapped input files.
In the NekRS input file, we add userobjects to compute average wall temperatures
and bulk fluid temperatures in axial layers with [NekBinnedSideAverage](https://cardinal.cels.anl.gov/source/userobjects/NekBinnedSideAverage.html#)
and [NekBinnedVolumeAverage](https://cardinal.cels.anl.gov/source/userobjects/NekBinnedVolumeAverage.html) objects.
We then output the results of these userobjects to CSV using
[SpatialUserObjectVectorPostprocessors](https://mooseframework.inl.gov/source/vectorpostprocessors/SpatialUserObjectVectorPostprocessor.html). For the time being, we turn off the calculation of these objects
until we are ready to describe the generation of heat transfer
coefficients in [#htc].

!listing /tutorials/gas_compact_cht/solid_nek.i
  start=UserObjects
  end=Outputs

### THM-MOOSE

In the previous section, we described a coupling of NekRS and MOOSE.
In this section, we describe the coupling of MOOSE and THM, a lower-order fluid solver.

#### Solid Input Files

The solid phase is again solved with the MOOSE heat conduction module, and
is described in the `solid_thm.i` input.
The solid input file is almost exactly the same as the heat conduction model used
for coupling to NekRS that has already been described, so we only focus on the differences.
Instead of creating a NekRS sub-application, we create a THM sub-application.

!listing /tutorials/gas_compact_cht/solid_thm.i
  block=MultiApps

Because THM is a 1-D code, we must perform an averaging operation on the surface
heat flux computed by MOOSE before it is sent to THM. This requires us to use
slightly different MOOSE transfers to couple THM and MOOSE. Instead of sending
a 2-D surface flux distribution, we now compute the wall average heat flux in layers
and send to THM with a [MultiAppGeneralFieldUserObjectTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppGeneralFieldUserObjectTransfer.html).
To receive temperature from THM, we use the same
[MultiAppGeneralFieldNearestNodeTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppGeneralFieldNearestNodeTransfer.html)
that we used when coupling to NekRS - but now, we are coupling a 1-D model to a 3-D model
instead of a 3-D model to a 3-D model. MOOSE's transfers are generally dimension agnostic.

!listing /tutorials/gas_compact_cht/solid_thm.i
  block=Transfers

Other postprocessing features in this MOOSE model are largely the same as in
the NekRS-MOOSE case; except to compute the wall heat flux to send to THM, we use
a [LayeredSideDiffusiveFluxAverage](https://mooseframework.inl.gov/source/userobject/LayeredSideDiffusiveFluxAverage.html),
which computes heat flux on a boundary given the temperature computed by MOOSE.

!listing /tutorials/gas_compact_cht/solid_thm.i
  block=UserObjects

#### Fluid Input Files

The fluid phase will be solved with THM, and is described in the `thm.i` file.
The THM input file is built using syntax specific to THM - we will only briefly
cover the syntax, and instead refer users to the [THM manuals](https://mooseframework.inl.gov/modules/thermal_hydraulics/index.html) for more information.
First, we define a number of constants at the beginning of the file and apply
some global settings. We set the initial conditions for pressure, velocity, and
temperature and indicate the fluid [!ac](EOS) object using
[IdealGasFluidProperties](https://mooseframework.inl.gov/source/userobjects/IdealGasFluidProperties.html).

!listing /tutorials/gas_compact_cht/thm.i
  end=AuxVariables

Next, we define the "components" in the domain. These components essentially consist
of the physics equations and boundary conditions solved by THM, but expressed
in THM-specific syntax. These components define single-phase flow in a pipe, an inlet
mass flowrate boundary condition, an outlet pressure condition, and heat transfer
to the pipe wall.

!listing /tutorials/gas_compact_cht/thm.i
  block=Components

Associated with these components are a number of closures, defined as materials.
We set up the Churchill correlation for the friction factor and the Dittus-Boelter
correlation for the convective heat transfer coefficient. Additional materials are
created to represent dimensionless numbers and other auxiliary terms, such as the
wall temperature. As can be seen here, the [Material](https://mooseframework.inl.gov/syntax/Materials/index.html)
system is not always used to represent quantities traditionally thought of
as "material properties."

!listing /tutorials/gas_compact_cht/thm.i
  block=Materials

THM computes the wall temperature to apply a boundary condition in the MOOSE
heat conduction module. To convert the `T_wall` material into an
auxiliary variable, we use the [ADMaterialRealAux](https://mooseframework.inl.gov/source/auxkernels/MaterialRealAux.html).

!listing /tutorials/gas_compact_cht/thm.i
  start=AuxVariables
  end=Materials

Finally, we set the preconditioner, a [Transient](https://mooseframework.inl.gov/source/executioners/Transient.html)
executioner, and set an Exodus output. We will run THM to convergence based on a tight
steady state relative tolerance of $10^{-8}$.

!listing /tutorials/gas_compact_cht/thm.i
  start=Preconditioning
  end=UserObjects

## Execution and Postprocessing

To run the coupled NekRS-MOOSE calculation,

```
mpiexec -np 500 cardinal-opt -i common_input.i solid_nek.i
```

This will run with 500 [!ac](MPI) processes.
To run the coupled THM-MOOSE calculation,

```
mpiexec -np 2 cardinal-opt -i common_input.i solid_thm.i --n-threads=2
```

which will run with 2 MPI ranks and 2 OpenMP threads per rank.

When the two simulations have completed, you will have created a number of different
output files:

- `solid_nek_out.e`, an Exodus file with the MOOSE solution for the NekRS-MOOSE calculation
- `solid_nek_out_nek0.e`, an Exodus file with the NekRS solution that was ultimately transferred
  in/out of MOOSE
- `ranstube0.f<n>`, a (custom format) NekRS output file with the NekRS solution; to convert
  to a format viewable in Paraview, consult the [NekRS documentation](https://nekrsdoc.readthedocs.io/en/latest/detailed_usage.html#visualizing-output-files)
- `solid_thm_out.e`, an Exodus file with the MOOSE solution for the THM-MOOSE calculation
- `solid_thm_out_thm0.e`, an Exodus file with the THM solution

All CSV output files are placed in the `csv` directory. In discussing
the results, we will refer to the nominal Dittus-Boelter THM simulations as the
"baseline" THM-MOOSE simulations. In [#htc] when we correct the Dittus-Boelter correlation
based on the NekRS-MOOSE simulations, we will refer to those THM-MOOSE simulations
are "corrected" THM-MOOSE simulations.

[uc_solid_temp] shows the solid temperature predicted for the NekRS-MOOSE simulation, the baseline
THM-MOOSE simulation, and the difference between the two (NekRS minus THM).
Temperatures are highest in the six compact regions, slightly downstream from the core
midplane due to the combined effects of convective heat transfer and the imposed sinusoidal
power distribution. The solid temperature distributions predicted with either NekRS
fluid coupling or THM fluid coupling agree very well with each other -
 the maximum solid temperature difference is 16.3 K.

!media unit_cell_cht_solid_temp.png
  id=uc_solid_temp
  caption=Solid temperature predicted for NekRS-MOOSE, baseline THM-MOOSE, and the difference between the two (NekRS minus baseline THM).
  style=width:80%;margin-left:auto;margin-right:auto

This code difference can be attributed to a number of different sources:

- Because THM averages the solid heat flux along the coolant channel perimeter,
  THM is incapable of representing the azimuthal variation in heat flux that
  occurs due to the power production in the six fuel compacts.
- NekRS uses a wall-resolved methodology, whereas THM uses the Dittus-Boelter correlation.
  Assuming the maximum temperature difference could be solely attributed to a
  difference in Nusselt number, a 16.3 K difference is equivalent to about a 10%
  difference in convective heat transfer coefficient. Errors as high as 25%
  may be observed when using a simple model such as the Dittus-Boelter correlation [!cite](incropera),
  and a 10% difference is well within the realm of the data scatter used to
  originally fit the Dittus-Boelter model [!cite](winterton)

[uc_solid_temp2] shows the solid temperature predictions on the axial midplane
for the NekRS-MOOSE, the baseline THM-MOOSE simulation, and the difference
between the two (NekRS minus baseline THM). [uc_solid_temp2] shows with greater clarity
that the highest temperatures are observed in the fuel compacts,
and that the inability of THM to resolve the angular variation in wall heat flux
contributes to some spatial variation of the solid temperature difference along $x-y$ planes.

!media solid_unit_cell_midplane.png
  id=uc_solid_temp2
  caption=Solid temperature predicted along the axial mid-plane for NekRS-MOOSE, baseline THM-MOOSE, and the difference between the two (NekRS minus baseline THM).
  style=width:80%;margin-left:auto;margin-right:auto

Given the limitations of the 1-D area-averaged equations in THM, the temperature differences observed
in [uc_solid_temp] and [uc_solid_temp2] suggest that some differences between
NekRS and THM could reasonably be ascribed to differences in spatial resolution
and small differences in Nusselt number. A common multiscale use case of
NekRS is the generation of closures for coarse-mesh thermal-fluid tools. In the next
section, we will describe how Cardinal can be used to generate these heat transfer coefficients.

## Heat Transfer Coefficients
  id=htc

In this section, we use Cardinal to
correct the THM closures to better reflect the unit cell geometry and thermal-fluid conditions
of interest. A correction is only computed at the single
Reynolds and Prandtl numbers characterizing the unit cell ($Re=223214$ and $Pr=0.655$). That is, a new coefficient $c$ is computed for a Dittus-Boelter type correlation with the same functional dependence on $Re$ and $Pr$,

\begin{equation}
\label{eq:nek_db}
Nu=cRe^{0.8}Pr^{0.4}
\end{equation}

The heat transfer coefficient was defined in [eq:htc2]. We simply re-run the NekRS case
and activate the `[UserObjects]` and `[VectorPostprocessors]` blocks (to save time, you can
restart your NekRS case from one of the output files generated in the first part of this
tutorial, making sure to comment out the temperature IC in the `ranstube.udf` because you'd
now be loading temperature). Once you re-run the inputs, you will have CSV files that
contain $q_i^{''}$, $T_{\text{wall},i}$, and $T_{\text{bulk},i}$ in each axial layer that
you can write a simple Python script to calculate $h_i$ and convert to a Nusselt number.

From the results of this additional simulation,
[htc_fig] shows the Nusselt number as a function of distance from the channel
inlet computed by NekRS as a dashed line. A single average Nusselt number
from the NekRS simulation is obtained as an axial average over the entire
NekRS solution. Also shown for comparison is the Nusselt number predicted by
the Dittus-Boelter correlation. NekRS predicts an average Nusselt number of 336.35,
whereas the Dittus-Boelter correlation predicts a Nusselt number of 369.14; that
is, the average NekRS Nusselt number is 8.9% smaller than the Dittus-Boelter correlation,
which agrees very well with the qualitative assessment surrounding the
discussion on [uc_solid_temp] that the leading contributor to a difference
between the solid temperature with a NekRS fluid coupling vs. a THM fluid coupling
could be ascribed to a small difference in convective heat transfer coefficient.

!media axial_Nu.png
  id=htc_fig
  caption=Nusselt number as a function of distance from the inlet for NekRS and the Dittus-Boelter correlation.
  style=width:50%;margin-left:auto;margin-right:auto

The NekRS simulation suggests a new coefficient $c=0.021$ for [eq:nek_db].
The Dittus-Boelter correlation is updated in the THM input file by simply changing the
`0.023` coefficient in the Nusselt number correlation to `0.021`.

!listing /tutorials/gas_compact_cht/thm.i
  block=Nu_mat

Then, the THM-MOOSE simulations repeated with the NekRS-informed closure.
[uc_solid_temp3] compares the solid temperature between NekRS-MOOSE with the corrected
THM-MOOSE simulation. With the corrected Nusselt number correlation, the maximum
solid temperature difference between NekRS-MOOSE and corrected THM-MOOSE is only 10.3 K.
Due to the use of an axially-constant Nusselt number in THM, a perfect agreement between
NekRS-MOOSE and corrected THM-MOOSE cannot be expected. The solid temperature
difference will tend to be largest in regions where the local Nusselt number
(dashed red line in [htc_fig]) deviates most from the global average (solid red line
in [htc_fig]). This effect is observed near the outlet of the domain, where the local
Nusselt number is less than half of the average Nusselt number, causing the highest
deviation between NekRS-MOOSE and corrected THM-MOOSE. On average, the magnitude of the temperature
difference is only 5.3 K.

!media unit_cell_cht_solid_temp_corrected.png
  id=uc_solid_temp3
  caption=Solid temperature predicted for NekRS-MOOSE, corrected THM-MOOSE, and the difference between the two (NekRS minus corrected THM).
  style=width:80%;margin-left:auto;margin-right:auto

[uc_solid_temp4] shows the solid temperature predictions on the axial midplane for
the NekRS-MOOSE simulation, the corrected THM-MOOSE simulation, and the difference
between the two (NekRS minus corrected THM). Changing the Nusselt number model in THM
only affects the wall temperature applied as a boundary condition in THM;
because THM solves 1-D equations, the radial variation in the error will still
be present (although of a smaller magnitude due to the temperature shift induced by a change in wall temperature).

!media solid_unit_cell_midplane_corrected.png
  id=uc_solid_temp4
  caption=Solid temperature predicted along the axial  midplane for NekRS-MOOSE, corrected THM-MOOSE, and the difference between the two (NekRS minus corrected THM).
  style=width:80%;margin-left:auto;margin-right:auto

Cardinal's seamless capabilities for generating coarse-mesh closures using NekRS has been
demonstrated. Now, just to present a few remaining results (all obtained with the corrected
THMN-MOOSE simulation).
[uc_fluid_temp] shows the fluid temperature predicted by NekRS-MOOSE and the corrected THM-MOOSE.
Also shown for context is the solid temperature in a slice through the domain, on a
different color scale than the fluid temperature. The NekRS simulation resolves the fluid
thermal boundary layer, whereas the THM model uses the Dittus-Boelter correlation to
represent the temperature difference between the heated wall and the bulk; therefore,
the fluid temperature shown in [uc_fluid_temp] for THM is the area-averaged fluid
temperature. The wall temperature predicted by NekRS follows a similar distribution
as the heat flux, peaking slightly downstream of the midplane due to the combined
effects of convective heat transfer and the imposed sinusoidal power distribution.

!media unit_cell_cht_fluid_temp.png
  id=uc_fluid_temp
  caption=Fluid temperature predicted for NekRS-MOOSE and corrected THM-MOOSE.
  style=width:80%;margin-left:auto;margin-right:auto

Finally, [unit_cell_cht_temps] compares the solutions for radially-averaged temperatures
along the flow direction. As already discussed, the solid temperatures
match very well between NekRS-MOOSE and THM-MOOSE and match the expected behavior
for a sinusoidal power distribution. The fluid bulk temperature increases along
the flow direction, with a faster rate of increase where the power density is highest.

!media unit_cell_cht_temps.png
  id=unit_cell_cht_temps
  caption=Radially-averaged temperatures for the NekRS-MOOSE and corrected THM-MOOSE simulations.
  style=width:50%;margin-left:auto;margin-right:auto
