# Tutorial 1C: Conjugate Heat Transfer for Turbulent Pin Bundle Flow
  id=tutorial1c

In this tutorial, you will learn how to:

- Couple NekRS to MOOSE for [!ac](CHT)
- Extract velocity and turbulent viscosity from a NekRS flow-only restart file
- Swap out NekRS for a different thermal-fluid MOOSE application, THM
- Compute heat transfer coefficients with NekRS

!alert! note
This tutorial makes use of the following major Cardinal classes:

- [NekRSMesh](/mesh/NekRSMesh.md)
- [NekTimeStepper](/timesteppers/NekTimeStepper.md)
- [NekRSProblem](/problems/NekRSProblem.md)

We recommend quickly reading this documentation before proceeding
with this tutorial.
This tutorial also requires you to download some mesh files from Box.
Please download the files from the `gas_compact_cht` folder
[here](https://anl.app.box.com/s/irryqrx97n5vi4jmct1e3roqgmhzic89/folder/141527707499)
and place these files within the same directory structure in
`tutorials/gas_compact_cht`.
!alert-end!

This tutorial describes how to use Cardinal to perform [!ac](CHT) coupling of NekRS
to MOOSE for turbulent flow modeling in a unit cell representation of a prismatic
gas reactor assembly. Two different thermal-hydraulics tools are used to model the
fluid domain, depending on the desired resolution:

- NekRS, which will solve the wall-resolved $k$-$\tau$ model
- THM, which will solve the 1-D area-averaged Navier-Stokes equations; THM is
  a 1-D systems level code based on MOOSE that essentially contains
  all the single-phase physics in RELAP-7 [!cite](relap7)

This tutorial will demonstrate how Cardinal can be used to generate
a convective heat transfer coefficient that can be input into another thermal-fluid
MOOSE application. This process will be shown using THM, which can readily be extended
to other MOOSE-based thermal-fluid codes.

This tutorial was developed with support from the NEAMS Thermal Fluids
Center of Excellence. A technical report [!cite](novak_coe) describing the physics models,
mesh refinement studies, and auxiliary analyses provides additional context
and application examples beyond the scope of this tutorial.

## Geometry and Computational Model

The geometry consists of a unit cell of a [!ac](TRISO)-fueled
gas reactor compact, loosely based on a point design available in the literature
[!cite](sterbentz).
A top-down view of the geometry is shown in
[unit_cell]. The fuel is cooled by helium flowing in a cylindrical channel
of diameter $d_c$. Cylindrical fuel compacts containing randomly-dispersed
[!ac](TRISO) particles at 15% packing fraction
are arranged around the coolant channel in a triangular
lattice; the distance between the compact and coolant channel centers
is $p_{cf}$. The diameter of the fuel compact cylinders is $d_f$.
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
| Coolant channel diameter | 1.6 |
| Fuel compact diameter | 1.27 |
| Fuel-to-coolant center distance | 1.628 |
| Height | 160 |
| TRISO kernel radius | 214.85e-4 |
| Buffer layer radius | 314.85e-4 |
| Inner PyC layer radius | 354.85e-4 |
| Silicon carbide layer radius | 389.85e-4 |
| Outer PyC layer radius | 429.85e-4 |

### Heat Conduction Model

!include steady_hc.md

The solid mesh is shown in [solid_mesh]; the only sideset defined in the domain
is the coolant channel surface. The [!ac](TRISO) particles are homogenized into
the compact regions - all material properties in the heterogeneous regions
are taken as volume averages of the various constituent materials.
To simplify the specification of
material properties, the solid geometry uses a length unit of meters.

!media compact_solid_mesh.png
  id=solid_mesh
  caption=Mesh for the solid heat conduction model
  style=width:60%;margin-left:auto;margin-right:auto

The volumetric power density $\dot{q}_q$ is set to a sinusoidal function of $z$,

\begin{equation}
\dot{q}_s=q_0\sin{\left(\frac{\pi z}{H}\right)}
\end{equation}

where $q_0$ is a coefficient used to ensure that the total power produced is 38 kW
and $H$ is the domain height. The power is uniform
in the radial direction in each compact, effectively homogenizing the [!ac](TRISO)
particles into the matrix.

On the coolant channel surface, a Dirichlet temperature is provided by NekRS/THM.
All other boundaries are insulated. We will run the solid model first, so we must specify
an initial condition for the wall temperature, which we simply set to a linear variation
between the inlet and the outlet based on the nominal temperature rise.

### NekRS Model

!include ktau.md

The inlet mass flowrate is 0.0905 kg/s; with the channel diameter of 1.6 cm and material
properties of helium, this results in a Reynolds number of 223214 and a Prandtl number
of 0.655. This highly-turbulent flow results in extremely thin momentum and thermal boundary
layers on the no-slip surfaces forming the periphery of the coolant channel. In order to
resolve the near-wall behavior with a wall-resolved model, an extremely fine mesh is
required in the NekRS simulation. To accelerate the overall coupled [!ac](CHT) case
that is of interest in this tutorial, the NekRS model is split into a series of calculations:

1. We first run a partial-height, periodic flow-only case
   to obtain converged pressure, velocity, and turbulent viscosity distributions.
2. Then, we extrapolate the velocity and turbulent viscosity to the full-height case.
3. Finally, we use the converged, full-height velocity and turbulent viscosity distributions
   to transport a temperature passive scalar in a conjugate heat transfer calculation with MOOSE.

As a rough estimate, solving the coupled mass-momentum equations requires about an
order of magnitude more compute time than a passive scalar equation in NekRS, assuming
an identical time step size. Therefore, by "freezing" the velocity and $\mu_T$ solutions,
we can dramatically reduce the total cost of the coupled conjugate heat transfer calculation.

The periodic flow model has a height of $\frac{H}{10}$ and takes about 10,000 core hours
to reach steady state. The extrapolation from $\frac{H}{10}$ to $H$ then is a simple postprocessing
operation. Because steps 1 and 2 are done exclusively using NekRS, and the periodic flow solve
takes a very long time to run (from the perspective of a tutorial, at least), we omit steps 1
and 2 from this tutorial. Instead, we begin straight away from a full-height periodic restart
file with the $k$-$\tau$ [!ac](RANS) model. This restart file is the
`periodic_flow_solve.f00001` file.

For the conjugate heat transfer case, we will load this restart file, compute $k_T$ from the
loaded solutions for $k$ and $\tau$, and then transport temperature with coupling to MOOSE
heat conduction. Let's now describe the NekRS input files needed for the passive scalar solve.
These files are:

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
This input sets up a nondimensional passive scalar solution, loading pressure, velocity,
$k$, and $\tau$ from a restart file. In order to "freeze," or turn off the pressure, velocity,
$k$, and $\tau$ solves, we set `solver = none` in the `[VELOCITY]`, `[SCALAR01]` ($k$ passive scalar),
and `[SCALAR02]` ($\tau$ passive scalar) blocks. In the nondimensional formulation,
the "viscosity" becomes $1/Re$, where $Re$ is the Reynolds number, while the
"thermal conductivity" becomes $1/Pe$, where $Pe$ is the Peclet number. There nondimensional
numbers are used to set various diffusion coefficients in the governing equations
with syntax like `-223214`, which is equivalent in NekRS syntax to $\frac{1}{223214}$.
The only equation that NekRS will solve is for temperature.

!listing /tutorials/gas_compact_cht/ranstube.par

Next, the `.udf` file is used to setup initial conditions and define how
$k_T$ should be computed based on $Pr_T$ and the restart values of $k$ and $\tau$.
In `turbulent_props`, a user-defined function, we use $k_f$ from the input file
in combination with the $Pr_T$ and $mu_T$ (read from the restart file later in
the `.udf` file) to adjust the total diffusion coefficient on temperature to
$k_f+k_T$ according to [eq:PrT]. This adjustment must happen on device, in a new GPU kernel we name
`scalarScaledAddKernel`. This kernel will be defined in the `.oudf` file; we
instruct the JIT compilation to compile this new kernel by calling
`udfBuidlKernel`.

Then, in `UDF_Setup` we set an initial condition for fluid
temperature (the first scalar in the `nrs->cds->S` array that holds all the
scalars). In this function, we also store the value of $\mu_T$ computed in the
restart file based on [eq:mu_ktau].

!listing /tutorials/gas_compact_cht/ranstube.udf language=cpp

In the `.oudf` file, we define boundary conditions for temperature and also
the form of the `scalarScaledAdd` kernel that we use to compute $k_T$.

!listing /tutorials/gas_compact_cht/ranstube.oudf language=cpp


### THM Model

## CHT Coupling
  id=cht

### NekRS-MOOSE

### THM-MOOSE
