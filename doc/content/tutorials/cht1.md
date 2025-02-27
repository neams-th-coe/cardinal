# Conjugate Heat Transfer for Reflector Bypass Flow

In this tutorial, you will learn how to:

- Couple NekRS with MOOSE for [!ac](CHT) in a pebble bed reactor reflector block
- Propagate scalar values into the [!ac](OCCA) GPU kernels

To access this tutorial,

```
cd cardinal/tutorials/fhr_reflector
```

This tutorial also requires you to download
some mesh files and restart files from Box. Please download the files
from the `fhr_reflector` folder [here](https://anl.app.box.com/s/irryqrx97n5vi4jmct1e3roqgmhzic89/folder/141527707499)
and place these files within the same directory structure
in `tutorials/fhr_reflector`.

This tutorial was developed with funding from the NRIC [!ac](VTB). You can
find additional context on this model in our conference publication
[!cite](novak_ans_2021).

!alert! note title=Computing Needs
This tutorial requires [!ac](HPC) resources to run.
!alert-end!

## Geometry and Computational Model
  id=model

The pebble region in the [!ac](PB-FHR) is enclosed by an outer graphite reflector.
In order to keep the graphite reflector within
allowable design temperatures, the reflector contains several bypass flow
paths so that a small percentage of the coolant flow can
maintain the graphite within allowable temperature ranges. This
bypass flow
is important to quantify so that accurate
estimates of core cooling and reflector temperatures can be obtained. By repeating
these calculations for a range of Reynolds numbers and geometries, this model
can be used to provide friction factor and Nusselt number correlations.

This section describes the [!ac](PB-FHR) reflector geometry and our computational model.
A top-down view of the [!ac](PB-FHR) reactor is shown in [top_down], with important
specifications summarized in [table1].
The center region
is the pebble bed core, which is surrounded by two rings of graphite reflector blocks,
staggered in a brick-like fashion. The nominal
gap size between blocks is 0.002 m, but to ease meshing and make model depiction easier,
the gap between blocks is increased to 0.006 m
(which could be interpreted as an end-of-life, maximum-swelling condition
[!cite](novak2021)).
Each ring contains 24 blocks. In black is shown the core barrel.
To form the entire axial height of the reflector, rings of blocks are stacked
vertically. The entire core height is 5.3175 m, or
about 10 vertical rings of blocks. Additional structures are present as well,
but are not considered here.

!media top_down.png
  id=top_down
  caption=Top-down schematic of the [!ac](PB-FHR) reactor core.
  style=width:40%;margin-left:auto;margin-right:auto;halign:center

!table id=table1 caption=Geometric and operating conditions relevant to reflector block modeling, based on [!cite](shaver)
| Parameter | Value |
| :- | :- |
| Pebble core radius | 2.6 m |
| Inner block radial thickness | 0.3 m |
| Outer block radial thickness | 0.4 m |
| Barrel thickness | 0.022 m |
| Gap between blocks | 0.006 m |
| Gap between blocks and barrel | 0.02 m |
| Block height | 0.52 m |
| Core inlet temperature | 600&deg;C |
| Core outlet temperature | 700&deg;C |

FLiBe flows through the pebble bed, through small gaps between reflector
blocks, and between small gaps between the reflector blocks and the barrel. These bypass paths are
shown as yellow dashed lines in [top_down]. Due to the top-down orientation of [top_down],
flow paths radially (i.e. from lower to higher $r$ at a fixed $z$ between stacks
of reflector blocks) are not shown. While
such radial bypass paths contribute to bypass flow, they are not considered here.

The coupled NekRS-MOOSE simulation is conducted
for a single ring of reflector blocks (i.e. a height of 0.52 m), with azimuthal symmetry assumed to further
reduce the domain to half of an inner ring block, half of an outer ring block,
and the vertical bypass flow paths between the blocks and the barrel. This
computational domain is shown outlined with a red dotted line in [top_down].

### Heat Conduction Model

The MOOSE heat transfer module is used to solve for
[energy conservation in the solid](theory/heat_eqn.md).
A fixed heat flux of 5 kW/m$^2$ is imposed on the block surface facing the pebble bed.
On the surface of the barrel, a heat convection boundary condition is imposed,

\begin{equation}
\label{eq:hfc}
q^{''}=h\left(T_s-T_\infty\right)
\end{equation}

where $q^{''}$ is the heat flux, $h$ is the convective heat transfer coefficient, and $T_\infty$
is the far-field ambient temperature.
At fluid-solid interfaces, the solid temperature is imposed as a Dirichlet condition,
where NekRS computes the surface temperature. All other solid boundaries are treated as insulated.

### NekRS Model

NekRS is used to solve the [incompressible Navier-Stokes equations](theory/ins.md)
in [non-dimensional form](theory/nondimensional_ns.md).
In this tutorial, the following characteristic scales are selected:

- $L_{ref}=0.006$ m, such that the block gap width is unity
- $u_{ref}=0.0575$ m/s, which corresponds to a Reynolds number of 100
- $T_{ref} = 923.15$ K, the inlet temperature
- $\Delta T=10$ K, an arbitrary selection

A Reynolds number of 100 corresponds to a bypass fraction of about 7%, considering
that $Re$ can be written equivalently as

\begin{equation}
\label{eq:Re2}
Re\equiv\frac{\dot{m}L_{ref}}{A\mu}
\end{equation}

where $\dot{m}$ is the mass flowrate (a fraction of the total core mass flowrate, distributed
among 48 half-reflector blocks) and $A$ is the inlet flow area.

The only characteristic scale that requires additional comment is $\Delta T$ -
this parameter truly is arbitrary, since it does not appear in the definitions of
$Re$ or $Pe$ (*and*, none of the initial conditions/kernels in NekRS's case files
assume any range for temperature). That is, if the converged solution has an inlet temperature of 400 K
and an outlet temperature of 500 K, setting $\Delta T=100$ means that $T^\dagger$ will
range from 0 to 1. Setting a different value, such as $\Delta T=50$, means that
$T^\dagger$ will instead range from 0 to 2. The only scenario where $\Delta T$ is
of consequences is if you are setting initial conditions for temperature or have
imposed a heat source yourself within the `.oudf` file.

At the inlet, the fluid temperature is taken as 650&deg;C, or the nominal
median fluid temperature. The inlet velocity is set to a uniform value such that the Reynolds number is 100.
At the outlet, a zero pressure is imposed. On the $\theta=0^\circ$ boundary (i.e. the $y=0$ boundary), symmetry is imposed
such that all derivatives in the $y$ direction are zero. All other boundaries are treated as no-slip.

!alert note
The $\theta=7.5^\circ$ boundary (i.e. $360^\circ$ divided by 24 blocks, divided
in half because we are modeling only half a block) should also be imposed as a symmetry
boundary in the NekRS model. However, at the time when we first made this tutorial,
NekRS only supported symmetry
boundaries for sidesets aligned with the $x$, $y$, and $z$ coordinate
axes. Here, a no-slip boundary condition is used instead, so the correspondence of
the NekRS computational model to the depiction in [#model] is imperfect. This is no
longer a restriction in NekRS.

At fluid-solid interfaces, the heat flux is imposed as a Neumann condition, where MOOSE
computes the surface heat flux.

Because the NekRS mesh contains very small elements in the fluid phase, fairly small time
steps are required to meet [!ac](CFL) conditions related to stability. Therefore,
the approach to the coupled, pseudo-steady [!ac](CHT) solution can be
accelerated by obtaining initial conditions from a pure conduction simulation. Then, the
initial conditions for the [!ac](CHT) simulation will begin from the temperature obtained
from the conduction simulation, with a uniform axial velocity and zero pressure.
The process to run Cardinal in conduction mode is described in [#part1], while the process
to run Cardinal in [!ac](CHT) mode is described in [#part2].

## Meshing
  id=meshes

Cubit [!cite](cubit) is used to
programmatically create meshes with user-defined geometry and customizable
boundary layers. Journal files, or Python-scripted Cubit inputs, are used
to create meshes in Exodus II format. The MOOSE framework accepts meshes in
a wide range of formats that can be generated with many commercial and free
meshing tools; other tutorials will make meshes natively within MOOSE.

### Solid Mesh
  id=solid_mesh

The Cubit script used to generate the solid mesh is
shown below. To run this script yourself, open Cubit and click on 'Play Journal File' 
(the play button on a sheet of paper icon), and then find and open your file. Also, you will need
to update the `directory` variable to point to the tutorials directory on your machine.

!listing /tutorials/fhr_reflector/meshes/solid.jou language=python

The solid mesh (before a series of refinements) is shown below; the boundary names are illustrated
by showing only the highlighted surface to which each boundary corresponds.
A unique block ID is used for the set of elements
corresponding to the inner ring, outer ring, and barrel. Material properties in MOOSE
are typically restricted by block, and setting three separate IDs allows us to set
different properties in each of these blocks.

!media solid_mesh.png
  id=solid_mesh
  caption=Solid mesh for the reflector blocks and barrel and a subset of the boundary names, before a series of mesh refinements
  style=halign:center

Unique boundary names are set for each boundary to which we will apply a unique
boundary condition; we define the boundaries on the top and bottom of the block,
the symmetry boundaries,
and boundaries at the interface between the
reflector and the bed and on the barrel surface.

One convenient aspect of MOOSE is that the same elements
can be assigned to more than one boundary ID. To help in applying heat flux and
temperature boundary conditions between NekRS and MOOSE, we define another boundary
that contains all of the fluid-solid interfaces through which we will exchange
heat flux and temperature, as `fluid_solid_interface`.

### Fluid Mesh
  id=fluid_mesh

The Cubit script used to generate the fluid mesh is shown below.
To run this script yourself, you will need to update
the `directory` variable to point to the tutorials directory on your machine.

!listing /tutorials/fhr_reflector/meshes/fluid.jou language=python

The fluid mesh is shown below; the boundary names are illustrated towards
the right by showing the highlighted surface to which each boundary corresponds.
While the names of the surfaces are shown,
NekRS does not directly use these names - rather, NekRS assigns boundary
conditions based on the boundary ID.
An important restriction in NekRS is that the boundary IDs be ordered
sequentially beginning from 1.

!media fluid_mesh.png
  id=fluid_mesh
  caption=Fluid mesh for the FLiBe flowing around the reflector blocks, along with boundary names and IDs. It is difficult to see, but the `porous_inner_surface` boundary corresponds to the thin surface at the interface between the reflector region and the pebble bed.
  style=halign:center

One strength of Cardinal for [!ac](CHT) applications
is that the fluid and solid meshes do not need to share nodes on a common surface; MOOSE
mesh-to-mesh data interpolations can communicate between surfaces with different refinement and position in
space; meshes may even overlap or share no nodes at all, such as
for curvilinear applications. [zoom_mesh] shows
a zoom-in of the two mesh files (for the fluid and solid phases). Each phase
can be meshed according to its physics requirements.

!media zoom_mesh.png
  id=zoom_mesh
  caption=Zoomed-in view of the fluid and solid meshes, overlaid in Paraview. Lines are element boundaries.
  style=width:50%;margin-left:auto;margin-right:auto;halign:center

Because NekRS uses a custom binary mesh format with a `.re2` extension, the `exo2nek` utility
must be used to convert an Exodus II format to `.re2` format.
For more information, please consult the [tools documentation](https://nekrsdoc.readthedocs.io/en/latest/detailed_usage.html).

## Part 1: Initial Conduction Coupling
  id=part1

In this section, NekRS and MOOSE are coupled for conduction heat transfer in the solid reflector
blocks and barrel, and through a stagnant fluid. The purpose of this stage of
the analysis is to obtain a restart file for use as an initial condition in [#part2] to accelerate the NekRS
calculation for [!ac](CHT).
All input files for this stage are present in the
`tutorials/fhr_reflector/conduction` directory. The following sub-sections describe these files.

### Solid Input Files

The solid phase is solved with the MOOSE heat transfer module, and is described in the `solid.i` input.
At the top of this file, the core heat flux is defined as a variable local to the file.

!listing /tutorials/fhr_reflector/conduction/solid.i
  end=Mesh

Next, the solid mesh is
specified by pointing to the Exodus mesh.

!listing /tutorials/fhr_reflector/conduction/solid.i
  start=Mesh
  end=Variables

The heat transfer module will solve for temperature, which is defined as a nonlinear
variable.

!listing /tutorials/fhr_reflector/conduction/solid.i
  start=Variables
  end=AuxVariables

The [Transfer](Transfers/index.md)
 system is used to communicate auxiliary variables across applications;
a boundary heat flux will be computed by MOOSE and applied as a boundary condition in NekRS.
In the opposite direction, NekRS will compute a surface temperature that will be applied as
a boundary condition in MOOSE. Therefore, both the flux (`flux`) and surface temperature
(`nek_temp`) are declared as auxiliary variables. The solid app will *compute* `flux`,
while `nek_temp` will simply *receive* a solution from NekRS.

!listing /tutorials/fhr_reflector/conduction/solid.i
  start=AuxVariables
  end=Functions

An initial condition is set for `nek_temp` using a function.

!listing /tutorials/fhr_reflector/conduction/solid.i
  start=Functions
  end=Kernels

Next, the governing equation solved by MOOSE is specified with the `Kernels` block.

!listing /tutorials/fhr_reflector/conduction/solid.i
  start=Kernels
  end=BCs

Next, the boundary conditions on the solid are applied. On the fluid-solid interface,
a [MatchedValueBC](MatchedValueBC.md)
 applies the value of the `nek_temp` receiver auxiliary variable
to the temperature in a strong Dirichlet sense. Insulated boundary conditions are applied on the `symmetry`,
`top`, and `bottom` boundaries. On the boundary at the bed-reflector interface, the
core heat flux is specified as a [NeumannBC](NeumannBC.md). Finally, on the surface of the barrel,
a heat flux of $h(T-T_\infty)$ is specified, where both $h$ and $T_\infty$ are specified
as material properties.

!listing /tutorials/fhr_reflector/conduction/solid.i
  start=BCs
  end=Materials

The [HeatConduction](HeatConduction.md)
 kernel requires a material property for the thermal conductivity;
material properties are also required for the heat transfer coefficient and far-field
temperature for the [ConvectiveHeatFluxBC](ConvectiveHeatFluxBC.md)
boundary condition. These material properties are specified
in the `Materials` block. Here, different values for thermal conductivity are used
in the graphite and steel.

!listing /tutorials/fhr_reflector/conduction/solid.i
  start=Materials
  end=MultiApps

Next, the [MultiApps](MultiApps/index.md)
 and [Transfers](Transfers/index.md)
blocks describe the interaction between Cardinal
and MOOSE. The MOOSE heat transfer module is here run as the main application, with
the NekRS wrapping run as the sub-application. We specify that MOOSE will run first on each
time step.

Three transfers are required to couple Cardinal and MOOSE; the first is a transfer
of surface temperature from Cardinal to MOOSE.
The second is a transfer of heat flux from
MOOSE to Cardinal.
And the third is a transfer of the total integrated heat flux from MOOSE
to Cardinal (computed as a postprocessor), which is then used internally by NekRS to re-normalize the heat flux (after
interpolation onto NekRS's [!ac](GLL) points).

!listing /tutorials/fhr_reflector/conduction/solid.i
  start=MultiApps
  end=Postprocessors

Next, postprocessors are used to compute the integral heat flux as a
[SideIntegralVariablePostprocessor](SideIntegralVariablePostprocessor.md).

!listing /tutorials/fhr_reflector/conduction/solid.i
  start=Postprocessors
  end=Executioner

Finally, a transient executioner is used for the solid phase.
The overall coupled simulation is considered
converged once the relative change in the solution between steps is less than $5\times10^{-4}$.
An output format of Exodus II is specified.

!listing /tutorials/fhr_reflector/conduction/solid.i
  start=Executioner

### Fluid Input Files
  id=fluid_model

The fluid phase is solved with NekRS.
The wrapping of NekRS as a MOOSE
application is specified in the `nek.i` file.

First, a local variable, `fluid_solid_interface`, is used to define all the boundary IDs through which NekRS is coupled
via [!ac](CHT) to MOOSE. A first-order mirror of the NekRS mesh
is constructed using the [NekRSMesh](NekRSMesh.md). By specifying the
`boundary` parameter, we are indicating that NekRS will be coupled via [!ac](CHT) through
boundaries 1, 2, and 7 to MOOSE.
In order for MOOSE's transfers
to correctly find the closest nodes in the solid mesh to corresponding nodes in this fluid mesh mirror, the entire mesh must be
scaled by a factor of $L_{ref}$ to return to dimensional units (because the coupled MOOSE
application is in dimensional units). This scaling is specified by the
`scaling` parameter.

!listing /tutorials/fhr_reflector/conduction/nek.i
  end=Problem

Next, the [NekRSProblem](NekRSProblem.md) specifies that NekRS simulations will be
run in place of MOOSE.
To allow conversion between a non-dimensional NekRS solve and a dimensional MOOSE coupled
heat conduction application, the characteristic scales used to establish the non-dimensional
problem are provided.

!listing /tutorials/fhr_reflector/conduction/nek.i
  start=Problem
  end=Executioner

Next, a [Transient](Transient.md) executioner
is specified with the [NekTimeStepper](NekTimeStepper.md), which allows NekRS
to control its own time stepping (except for synchronization points).

!listing /tutorials/fhr_reflector/conduction/nek.i
  start=Executioner
  end=Postprocessors

Finally, several postprocessors are included to perform
integrals and global min/max calculations over the NekRS domain for diagnostic purposes.
Here, the [NekHeatFluxIntegral](NekHeatFluxIntegral.md)
postprocessor computes $-k\nabla T\cdot\hat{n}$ over a boundary in the NekRS mesh. The
[NekVolumeExtremeValue](NekVolumeExtremeValue.md) postprocessors then compute the maximum
and minimum temperatures throughout the entire NekRS domain (i.e. not only on the [!ac](CHT)
coupling surfaces).

!listing /tutorials/fhr_reflector/conduction/nek.i
  start=Postprocessors

The `nek.i` input file only describes how NekRS is *wrapped* within the MOOSE framework;
each NekRS simulation requires additional files
that share the same case name `fluid` but with different extensions.
The additional NekRS files are:

- `fluid.re2`: Mesh file
- `fluid.par`: High-level settings for the solver, boundary
  condition mappings to sidesets, and the equations to solve
- `fluid.udf`: User-defined C++ functions for on-line postprocessing and model setup
- `fluid.oudf`: User-defined [!ac](OCCA) kernels for boundary conditions and
  source terms

First, begin with the `fluid.par` file.

!listing /tutorials/fhr_reflector/conduction/fluid.par

Here, a time step
of 0.025 (non-dimensional) is used; a NekRS output file is written every 100 time steps.
Because the purpose of this
simulation is only to obtain a reasonable initial condition, a low polynomial order
of 2 is used.

Next, the `[VELOCITY]` and `[PRESSURE]` blocks describe the solution of the
pressure Poisson equation and velocity Helmholtz equations. In the velocity block,
setting `solver = none` turns off the velocity solution; therefore, none of the
parameters specified here are used right now, so their description will be deferred
to [#part2]. Finally, the `[TEMPERATURE]` block describes the solution of the
temperature passive scalar equation. $\rho_fC_{p,f}$ is set to unity because the
solve is conducted in non-dimensional form, such that

\begin{equation}
\label{eq:nek1}
\rho^\dagger C_{p,f}^\dagger\equiv\frac{\rho_fC_{p,f}}{\rho_0C_{p,0}}=1
\end{equation}

The coefficient on the diffusive equation term in the [non-dimensional energy equation](theory/nondimensional_ns.md)
is equal to $1/Pe$. In NekRS, specifying `conductivity = -1500.5` is equivalent
to specifying `conductivity = 0.00066644` (i.e. $1/1500.5$), or a Peclet number of
1500.5.

Next, `residualTol` specifies the solver tolerance for the temperature equation
to $10^{-8}$. Finally, the `boundaryTypeMap` is used to specify the mapping of
boundary IDs to types of boundary conditions.
Boundaries 1, 2, and 7 are flux boundaries
(these boundaries will receive a heat flux from MOOSE), boundaries 3, 4, and 8 are
insulated, boundary 5 is a specified temperature, and boundary 6 is a zero-gradient
outlet. The actual assignment of values for these boundary conditions is then
performed in the `fluid.oudf` file.
Because this case does not have any user-defined
source terms in NekRS, these [!ac](OCCA) kernels are only used to apply boundary conditions.

!listing /tutorials/fhr_reflector/conduction/fluid.oudf language=cpp

Finally, the `fluid.udf` file contains user-defined C++ functions through
which other interactions with the NekRS solution are performed. Here, the `UDF_Setup` function
is called once at the very start of the NekRS simulation, and it is here that initial
conditions are applied.

!listing /tutorials/fhr_reflector/conduction/fluid.udf language=cpp

The initial condition is applied by looping over all
the [!ac](GLL) points and setting zero to each (recall that this is a non-dimensional
simulation, such that $T^\dagger=0$ corresponds to a dimensional temperature of $T_{ref}$).
Here, `nrs->cds->S` is the array holding the NekRS passive scalar solutions (of which
there is only one for this example).

### Execution and Postprocessing
  id=ep

To run the pseudo-steady conduction model, run the following:

```
mpiexec -np 48 cardinal-opt -i solid.i
```

which will run with 48 MPI ranks.
When the simulation has completed, you will have created a number of different output files:

- `fluid0.f<n>`, where `<n>` is a five-digit number indicating the output file number
  created by NekRS (a separate output file is written for each time step
  according to the settings in the `fluid.par` file). An extra program is required to visualize
  NekRS output files in Paraview; see the instructions [here](https://nekrsdoc.readthedocs.io/en/latest/detailed_usage.html#visualizing-output-files).
- `solid_out.e`, an Exodus II output file with the solid mesh and solution.
- `solid_out_nek0.e`, an Exodus II output file with the fluid mirror mesh
  and data that was ultimately transferred in/out of NekRS.

After converting the NekRS output files to a format viewable in Paraview, the simulation
results can be displayed. The solid temperature, surface heat flux, and fluid temperature
are shown below. Note that the fluid temperature is shown in nondimensional form based on
the selected characteristic scales. The image of the fluid solution is rotated so as to
better display the temperature variation around the inner reflector block.
The domain shown in [solid_steady_flux] exactly corresponds to the "mirror" mesh
constructed by [NekRSMesh](NekRSMesh.md).

!media fhr_reflector_solid_conduction.png
  id=solid_steady
  caption=Solid temperature for steady state conduction coupling between MOOSE and NekRS
  style=width:60%;margin-left:auto;margin-right:auto;halign:center

!media fhr_reflector_solid_conduction_flux.png
  id=solid_steady_flux
  caption=Solid surface heat flux for steady state conduction coupling between MOOSE and NekRS
  style=width:60%;margin-left:auto;margin-right:auto;halign:center

!media fhr_reflector_fluid_conduction.png
  id=fluid_steady
  caption=Fluid temperature (nondimensional) for steady state conduction coupling between MOOSE and NekRS
  style=width:60%;margin-left:auto;margin-right:auto;halign:center

The solid blocks are heated by the pebble bed along the bed-reflector interface, so the
temperatures are highest in the inner block. As can be seen in [solid_steady_flux], the
heat flux into the fluid is in some places positive (such as near the inner reflector block)
and is in other places negative (such as near the barrel) where heat leaves the system.

## Part 2: CHT Coupling
  id=part2

In this section, NekRS and MOOSE are coupled for [!ac](CHT) with fluid flow.
All input files for this stage of the analysis are present in the
`tutorials/fhr_reflector/cht` directory. The following sub-sections describe all of these files; for
brevity, most emphasis will be placed on input file setup that is different or extends the
conduction case in [#part1].

### Solid Input Files

The solid phase is again solved with the MOOSE heat transfer module; the input file
is largely the same except that the simulation is run for 400 time steps.

!listing /tutorials/fhr_reflector/cht/solid.i
  block=Executioner

### Fluid Input Files

The fluid phase is solved with NekRS. The
input file is largely the same as the conduction case, except that additional
postprocessors are added to query more aspects of the NekRS
solution. The postprocessors are shown below.

!listing /tutorials/fhr_reflector/cht/nek.i
  start=Postprocessors

We have added postprocessors to compute the average inlet pressure and the average
inlet and outlet mass flowrates. Like the [NekVolumeExtremeValue](NekVolumeExtremeValue.md)
postprocessor, these postprocessors
operate directly on NekRS's internal solution arrays to provide diagnostic information.
Because the outlet pressure is set to zero, `pressure_in` corresponds to the pressure
drop in the fluid.

As in [#part1], additional files are required to set up the NekRS simulation -
`fluid.re2`, `fluid.par`, `fluid.udf`, and `fluid.oudf`. These files are largely the
same as those used in the steady conduction model, so only the differences will be
emphasized here.
The `fluid.par` file is shown below. Here, `startFrom` provides a restart file (that
we generated from [#part1]),
`conduction.fld` and specifies that we only want to read temperature from the
file (by appending `+T` to the file name). We increase the polynomial order as well.

!listing /tutorials/fhr_reflector/cht/fluid.par

In the `[VELOCITY]` block, the density is set to unity, because the solve is conducted
in nondimensional form, such that

\begin{equation}
\label{eq:nondim_p}
\rho^\dagger\equiv\frac{\rho_f}{\rho_0}=1
\end{equation}

The coefficient on the diffusive term in the [non-dimensional momentum equation](theory/nondimensional_ns.md)
is equal to
$1/Re$. In NekRS, specifying `diffusivity = -100.0` is equivalent to specifying
`diffusivity = 0.001` (i.e. $1/100.0$), or a Reynolds number of 100.0. All other parameters
have similar interpretations as described in [#part1].

The `fluid.udf` file is shown below. The `UDF_Setup` function is again used to apply initial
conditions; because temperature is read from the restart file, only initial conditions on
velocity and pressure are required. `nrs->U` is an array storing the three components of
velocity (padded with length `nrs->fieldOffset`), while `nrs->P` is the array storing the
pressure solution.

!listing /tutorials/fhr_reflector/cht/fluid.udf language=cpp

This file also includes the `UDF_LoadKernels` function, which is used to propagate
quantities to variables accessible through [!ac](OCCA) kernels. The `kernelInfo`
object is used to define two variables - `Vz` and `inlet_T` that will be accessible
through the [!ac](GPU) kernels, eliminating some burden on the user if the problem
setup must be changed in multiple locations throughout the NekRS input files.

Finally, the `fluid.oudf` file is shown below. Because the velocity is enabled,
additional boundary condition functions must be specified.
In the `velocityDirichletConditions` function, the kernel variable `Vz` was defined in the `fluid.udf` file.
The other boundary conditions -
the Dirichlet temperature conditions and the Neumann heat flux conditions - are the
same as for the steady conduction case.

!listing /tutorials/fhr_reflector/cht/fluid.oudf language=cpp

### Execution and Postprocessing

To run the pseudo-steady [!ac](CHT) model, run the following:

```
$ mpiexec -np 48 cardinal-opt -i solid.i
```

The pressure and velocity distributions
are shown below, both in non-dimensional form.

!media fhr_pressure.png
  id=pressure_cht
  caption=Pressure (nondimensional) for [!ac](CHT) coupling between MOOSE and NekRS
  style=width:60%;margin-left:auto;margin-right:auto;halign:center

!media fhr_velocity.png
  id=velocity_cht
  caption=Velocity (nondimensional) for [!ac](CHT) coupling between MOOSE and NekRS
  style=width:60%;margin-left:auto;margin-right:auto;halign:center

The no-slip condition on the solid surface,
and the symmetry condition on the $y=0$ surface, are clear in [velocity_cht]. The pressure
loss is highest in the gap along the $\theta=7.5^\circ$ boundary due to the imposition of
no-slip conditions on both sides of the half-gap width.
