# Conjugate Heat Transfer for Flow Over a Pebble

In this tutorial, you will learn how to:

- Create a NekRS case
- Couple NekRS with MOOSE for [!ac](CHT) for laminar flow over a single heated pebble
- Solve NekRS in non-dimensional form while MOOSE solves in dimensional form

To access this tutorial,

```
cd cardinal/tutorials/pebble_1
```

!alert! note title=Computing Needs
No special computing needs are required for this tutorial.
!alert-end!

At a high level, Cardinal's wrapping of NekRS consists of:

1. Construct a "mirror" of the NekRS mesh ([NekRSMesh](https://cardinal.cels.anl.gov/source/mesh/NekRSMesh.html))
  through which data transfers occur
  with MOOSE. A
  [MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html)
   is created by copying the NekRS mesh into a format that
  all native MOOSE applications can understand.
2. Add [MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
  to represent the NekRS solution. In other words,
  if NekRS stores the temperature internally as an `std::vector<double>`,
  with each
  entry corresponding to a NekRS node, then a [MooseVariable](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
   is created that represents
  the same data, but which is exposed to the MOOSE input files.
3. Write multiphysics feedback fields in/out of NekRS's internal data structures.
   In other words, if NekRS represents a heat flux boundary condition internally
  as an `std::vector<double>`,
  this involves reading from a [MooseVariable](https://mooseframework.inl.gov/source/variables/MooseVariable.html) representing
  heat flux
  and writing into NekRS's internal vectors.

Cardinal developers have an intimate knowledge of how NekRS stores its
solution fields and mesh, so this entire process is automated for you!
Setting up a coupling of NekRS to MOOSE only requires a handful of user
specifications.

## Geometry

The domain consists of

## "Standalone" NekRS Case

First, we will create a NekRS simulation without considering any coupling to MOOSE yet. This
will allow us to describe how to build NekRS input files (though for more detailed
information, we refer you to the [NekRS documentation](https://nekrsdoc.readthedocs.io/en/latest/index.html)).
Here, we will just create a fluid-only model of the flow around the pebble, with a uniform heat flux
on the pebble surface (which we will later replace by a coupled heat conduction solver performed by MOOSE).

### Creating a .re2 Mesh

NekRS uses a mesh in a custom `.re2` format.

!alert note
NekRS has some restrictions on what constitutes a valid mesh:
- Mesh must have hexahedral elements in either Hex8 (8-node hexahedral) or Hex20 (20-node hexahedral) forms
- The sidesets must be numbered sequentially beginning from 1 (e.g. sideset 1, 2, 3, 4, ...)

If you have a mesh in Exodus or Gmsh format,
you can convert that mesh into `.re2` format using [tools that ship with Nek5000](https://nekrsdoc.readthedocs.io/en/latest/detailed_usage.html). To get these features, you will need to clone Nek5000 somewhere on your system
and then build the `exo2nek` tool.

```
git clone https://github.com/Nek5000/Nek5000.git
cd Nek5000/tools
./maketools exo2nek
```

Running the above will place a binary named `exo2nek` at `Nek5000/bin/exo2nek`. We recommend
adding this to your path.

Now that you have `exo2nek`, we are ready to convert your mesh into `.re2` format.
In the directory where you have your mesh file, type

```
exo2nek
```

and then follow the prompts. For this case, we have:

- 1 fluid exo file,
- Which is named `pebble.exo` (you only need to provide `pebble` as the name)
- 0 solid exo files
- 0 periodic surface pairs
- And we want the output file name to be `pebble.re2` (you only need to provide `pebble` as the output name)

This will create a NekRS-ready mesh named `pebble.re2`.

NekRS is a spectral element code, which means that the solution in each element is represented
as an $N$-th order Lagrange polynomial (in each direction).
An illustration for a 5th-order NekRS solution is shown in [gll] for a 2-D element. Each
red dot is a node ([!ac](GLL) quadrature). When you create a mesh for NekRS, you do not
see these [!ac](GLL) points in your starting Exodus/Gmsh mesh. Instead, they will be created
when you launch NekRS (in other words, you do not need to create unique meshes if you want
to run NekRS at different polynomial orders).

!media gll.png
  id=gll_mesh
  caption=Illustration of nodal positions for a $N=5$ polynomial solution.
  style=width:30%;margin-left:auto;margin-right:auto;halign:center

### Input Files

Aside from the mesh, the other NekRS case files are summarized as follows:

- `pebble.par`: High-level settings for the solver, boundary
  conditions, and the equations to solve
- `pebble.udf`: User-defined C++ functions for on-line postprocessing and model setup
- `pebble.oudf`: User-defined [!ac](OCCA) kernels for boundary conditions and
  source terms

The default case file naming scheme used by NekRS is to use a common "casename"
(in this case, `pebble`), appended by the different file extensions which represents
different parts of the input file setup.

#### .par File

The `.par` file is used to set up the high-level settings for the case.
This file consists of blocks (in square brackets) and parameters. The `[GENERAL]` block describes the
time stepping, simulation end control, and polynomial order.

!include /tutorials/pebble_1/pebble.par

Here, the NekRS case will be set up in [non-dimensional form](nondimensional_ns.md).
So, the time step in the `[GENERAL]` block indicates the time step size in non-dimensional form
(i.e. it is not in units of seconds). A NekRS output file is written every 100 time steps.
The `stopAt` and `numSteps` fields indicate when to stop the simulation (after 500 time steps).
We will use a polynomial order of $N=5$.

Next, the `[VELOCITY]` and `[PRESSURE]` blocks describe the solution of the
pressure Poisson equation and velocity Helmholtz equations.
The `[TEMPERATURE]` block describes the solution of the
temperature passive scalar equation. In these blocks, `residualTol` is used to indicate
the solver tolerance. In these blocks, you also specify the type of boundary conditions
to apply to each sideset (you only specify boundary conditions in the `[VELOCITY]` and
`[TEMPERATURE]` blocks, because the pressure and velocity solves are really indicating
together a solution to the momentum conservation equation).
The `boundaryTypeMap` is used to specify the mapping of
boundary IDs to types of boundary conditions. NekRS uses short character strings
to represent the type of boundary condition. For velocity, these boundary condition strings are:

- `v`: Dirichlet velocity
- `w`: No-slip wall
- `o`: Outflow velocity + Dirichlet pressure
- `symx`: symmetry in the $x$-direction
- `symy`: symmetry in the $y$-direction
- `symz`: symmetry in the $z$-direction
- `sym`: general symmetry boundary

For temperature, these boundary condition strings are:

- `t`: Dirichlet temperature
- `f`: Neumann flux
- `I`: insulated
- `symx`: symmetry in the $x$-direction
- `symy`: symmetry in the $y$-direction
- `symz`: symmetry in the $z$-direction
- `sym`: general symmetry boundary

In the `[VELOCITY]` block, the `density` parameter is used to specify density,
and `viscosity` is used to specify viscosity. When the Navier-Stokes equations
are written in [non-dimensional form](nondimensional_ns.md), the "density" becomes unity because

\begin{equation}
\label{eq:nondim_p}
\rho^\dagger\equiv\frac{\rho_f}{\rho_0}=1
\end{equation}

The "viscosity" becomes the coefficient on the
viscous stress term (which becomes $1/Re$, where $Re$ is the Reynolds number).
In NekRS, specifying `diffusivity = -100.0` is equivalent to specifying
`diffusivity = 0.001` (i.e. $1/100.0$), or a Reynolds number of 100.0.

In the `[TEMPERATURE]` block, `rhoCp` indicates the coefficient on
the time derivative term (volumetric specific heat). In non-dimensional form,
this term becomes unity because

\begin{equation}
\label{eq:nek1}
\rho^\dagger C_{p,f}^\dagger\equiv\frac{\rho_fC_{p,f}}{\rho_0C_{p,0}}=1
\end{equation}

The `conductivity` indicates the coefficient on the diffusion kernel, which in non-dimensional
form is equal to $1/Pe$, where $Pe$ is the Peclet number. In NekRS, specifying `conductivity = -1500.5` is equivalent
to specifying `conductivity = 0.00066644` (i.e. $1/1500.5$), or a Peclet number of
1500.5.

### Execution and Postprocessing

## NekRS-MOOSE Coupling

### Heat Conduction Model

The MOOSE heat conduction module is used to solve for
[energy conservation in the solid](theory/heat_eqn.md).
The mesh is generated using MOOSE's [SphereMeshGenerator](https://mooseframework.inl.gov/source/meshgenerators/SphereMeshGenerator.html).
You can either generate this mesh "online" as part of the simulation setup, or
we can create it as a separate activity and then load it (just as you can load any Exodus
mesh into a MOOSE simulation). We will do the latter here.

The mesh is specified in the `mesh.i` file.

!listing /tutorials/pebble_1/mesh.i

We can run this file in "mesh-only mode" to generate an Exodus
mesh with

```
cardinal-opt -i mesh.i --mesh-only
```

which will create a file name `mesh_in.e` which contains the mesh.
When you run a MOOSE input file in `--mesh-only` mode, you will see
some information printed to the screen which describes the sidesets and subdomains
in the mesh (you can also look at this information visually in tools like Paraview).
For example, we can see that this mesh has a sideset numbered "0" and a subdomain
numbered "0".

!include mesh_output.md

If we open this file in Paraview, we can also see the mesh, as shown in [one_pebble_mesh].
The surface of the pebble is sideset 0.

!media one_pebble_mesh.png
  id=one_pebble_mesh
  caption=Mesh used for the solid heat conduction.
  style=width:30%;margin-left:auto;margin-right:auto;halign:center

### NekRS Model

NekRS is used to solve the [incompressible Navier-Stokes equations](theory/ins.md)
in [non-dimensional form](theory/nondimensional_ns.md).
In this tutorial, the following characteristic scales are selected:

- $L_{ref}=0.06$ m, such that the pebble diameter is unity
- $u_{ref}=0.0575$ m/s, which corresponds to a Reynolds number of 100
- $T_{ref} = 923.15$ K, the inlet temperature
- $\Delta T=10$ K, typically selected such that a non-dimensional temperature of 1.0 indicates
  the bulk temperature rise


## CHT Coupling

### Solid Input Files

The solid phase is solved with the MOOSE heat conduction module, and is described in the `solid.i` input.
At the top of this file, the core heat flux is defined as a variable local to the file.
The value of this variable can then be used anywhere else in the input file
with syntax like `${thermal_conductivity}`, similar to bash syntax. You can also use
similar syntax to do math inside a MOOSE input file. For example, to populate
a location in the input file with two times the value held by the file-local variable
`thermal_conductivity`, you would write `${fparse 2 * thermal_conductivity}`.

!listing /tutorials/pebble_1/solid.i
  end=Mesh

Next, the solid mesh is specified by pointing to the Exodus mesh.

!listing /tutorials/pebble_1/solid.i
  block=Mesh

The heat conduction module will solve for temperature, which is defined as a nonlinear
variable.

!listing /tutorials/pebble_1/solid.i
  block=Variables

Next, the governing equation solved by MOOSE is specified with the `Kernels` block as the
[HeatConduction](https://mooseframework.inl.gov/source/kernels/HeatConduction.html)
 kernel plus the
[BodyForce](https://mooseframework.inl.gov/source/kernels/BodyForce.html) kernel, or

\begin{equation}
-\nabla\cdot(k\nabla T)-\dot{q}=0$
\end{equation}

Next, the boundary conditions on the solid are applied. On the fluid-solid interface,
a [MatchedValueBC](https://mooseframework.inl.gov/source/bcs/MatchedValueBC.html)
 applies the value of a variable named `nek_temp` (discussed soon) as a Dirichlet condition.

!listing /tutorials/pebble_1/solid.i
  start=BCs
  end=Materials

The [HeatConduction](https://mooseframework.inl.gov/source/kernels/HeatConduction.html)
 kernel requires a material property for the thermal conductivity.

!listing /tutorials/pebble_1/solid.i
  block=Materials

[DiffusionFluxAux](https://mooseframework.inl.gov/source/auxkernels/DiffusionFluxAux.html) auxiliary kernel is specified
for the `flux` variable in order to compute the flux on the `fluid_solid_interface` boundary.

!listing /tutorials/pebble_1/solid.i
  start=Kernels
  end=BCs


The [Transfer](https://mooseframework.inl.gov/syntax/Transfers/index.html)
 system is used to communicate auxiliary variables across applications;
a boundary heat flux will be computed by MOOSE and applied as a boundary condition in NekRS.
In the opposite direction, NekRS will compute a surface temperature that will be applied as
a boundary condition in MOOSE. Therefore, both the flux (`flux`) and surface temperature
(`nek_temp`) are declared as auxiliary variables. The solid app will *compute* `flux`,
while `nek_temp` will simply *receive* a solution from NekRS. The flux
is computed as a constant monomial field (a single value per element) due to the manner in
which material properties are accessible in auxiliary kernels in MOOSE. However, no
such restriction exists for receiving the temperature from NekRS, so we define
`nek_temp` as the default first-order Lagrange basis.

!listing /tutorials/pebble_1/solid.i
  start=AuxVariables
  end=Functions

In this example, the overall calculation workflow is as follows:

1. Run MOOSE heat conduction with a given surface temperature distribution from NekRS.
2. Send heat flux to NekRS as a boundary condition.
3. Run NekRS with a given surface heat flux distribution from MOOSE.
4. Send surface temperature to MOOSE as a boundary condition.

The above sequence is repeated until convergence. For the very first
time step, an initial condition should be set for `nek_temp`, because we will be running
the MOOSE heat conduction simulation first. An initial condition is set using an arbitrary
function guess.

!listing /tutorials/pebble_1/solid.i
  start=Functions
  end=Kernels

Next, the [MultiApps](https://mooseframework.inl.gov/syntax/MultiApps/index.html)
 and [Transfers](https://mooseframework.inl.gov/syntax/Transfers/index.html)
blocks describe the interaction between Cardinal
and MOOSE. The MOOSE heat conduction module is here run as the main application, with
the NekRS wrapping run as the sub-application. We specify that MOOSE will run first on each
time step. Allowing sub-cycling means that, if the MOOSE time step is 0.05 seconds, but
the NekRS time step set in the `.par` file is 0.02 seconds, that for every MOOSE time step, NekRS will perform
three time steps, of length 0.02, 0.02, and 0.01 seconds to "catch up" to the main
application. If sub-cycling is turned off, then the smallest time step among all the various
applications is used.

Three transfers are required to couple Cardinal and MOOSE; the first is a transfer
of surface temperature from Cardinal to MOOSE.
The second is a transfer of heat flux from
MOOSE to Cardinal.
And the third is a transfer of the total integrated heat flux from MOOSE
to Cardinal (computed as a postprocessor), which is then used internally by NekRS to re-normalize the heat flux (after
interpolation onto NekRS's [!ac](GLL) points).

!listing /tutorials/pebble_1/solid.i
  start=MultiApps
  end=Postprocessors

!alert note
For transfers between two native MOOSE applications, you can ensure
conservation of a transferred field using the `from_postprocessors_to_be_preserved` and
`to_postprocessors_to_be_preserved` options available to any class inheriting from
[MultiAppConservativeTransfer](https://mooseframework.inl.gov/moose/source/transfers/MultiAppConservativeTransfer.html).
However, proper conservation of a field within NekRS (which uses a completely different
spatial discretization from MOOSE) requires performing such conservations in NekRS itself.
This is why an integral postprocessor must explicitly be passed.

Next, postprocessors are used to compute the integral heat flux as a
[SideIntegralVariablePostprocessor](https://mooseframework.inl.gov/source/postprocessors/SideIntegralVariablePostprocessor.html).

!listing /tutorials/pebble_1/solid.i
  start=Postprocessors
  end=Executioner

Next, the solution methodology is specified. Although the solid phase only
includes time-independent kernels, the heat conduction is run as a transient because NekRS
ultimately must be run as a transient (NekRS lacks a steady solver). We choose
to omit the time derivative in the solid energy equation because we will reach
the converged steady state faster than if the solve had to also ramp up the solid
temperature from the initial condition.

The overall coupled simulation is considered
converged once the relative change in the solution between steps is less than $5\times10^{-4}$.
Finally, an output format of Exodus II is specified.

!listing /tutorials/pebble_1/solid.i
  start=Executioner

### Fluid Input Files
  id=fluid_model

The fluid phase is solved with NekRS.
The wrapping of NekRS as a MOOSE
application is specified in the `nek.i` file.
The fluid input file is quite minimal, as the specification
of the NekRS problem setup is mostly performed using the NekRS standalone input files.

First, a local variable, `fluid_solid_interface`, is used to define all the boundary IDs through which NekRS is coupled
via [!ac](CHT) to MOOSE. A first-order mirror of the NekRS mesh
is constructed using the [NekRSMesh](/mesh/NekRSMesh.md). By specifying the
`boundary` parameter, we are indicating that NekRS will be coupled via [!ac](CHT) through
boundaries 1, 2, and 7 to MOOSE.
In order for MOOSE's transfers
to correctly find the closest nodes in the solid mesh to corresponding nodes in this fluid mesh mirror, the entire mesh must be
scaled by a factor of $L_{ref}$ to return to dimensional units (because the coupled MOOSE
application is in dimensional units). This scaling is specified by the
`scaling` parameter.

!listing /tutorials/pebble_1/nek.i
  end=Problem

!alert note
Note that `fluid.re2` does not appear anywhere in `nek.i` - the `fluid.re2` file is
a mesh used directly by NekRS, while [NekRSMesh](/mesh/NekRSMesh.md) is a mirror of the boundaries in `fluid.re2`
through which boundary coupling with MOOSE will be performed.

Next, the [Problem](https://mooseframework.inl.gov/syntax/Problem/index.html)
 block describes all objects necessary for the actual physics solve; for
the solid input file, the default of [FEProblem](https://mooseframework.inl.gov/source/problems/FEProblem.html)
was implicitly assumed.  However, to replace MOOSE finite element calculations with NekRS
spectral element calculations, the [NekRSProblem](problems/NekRSProblem.md) class is used.
To allow conversion between a non-dimensional NekRS solve and a dimensional MOOSE coupled
heat conduction application, the characteristic scales used to establish the non-dimensional
problem are provided. The `casename` is used to supply the file name prefix for
the NekRS input files.

!listing /tutorials/pebble_1/nek.i
  start=Problem
  end=Executioner

!alert warning
These characteristic scales are used by Cardinal to dimensionalize the NekRS solution
into the units that the coupled MOOSE application expects. *You* still need to properly
non-dimensionalize the NekRS input files (to be discussed later).

Next, a [Transient](https://mooseframework.inl.gov/source/executioners/Transient.html) executioner
is specified. This is the same executioner used for the solid case, except now a
different time stepper is used - [NekTimeStepper](/timesteppers/NekTimeStepper.md).
This time stepper simply
reads the time step specified in NekRS's `.par` file (to be described shortly),
and converts it to dimensional form if needed. Except for synchronziation points
with the MOOSE application(s) to which NekRS is coupled, NekRS controls all of its own
time stepping.

An Exodus II output format is specified.
It is important to note that this output file only outputs the NekRS solution fields that have
been interpolated onto the mesh mirror; the solution over the entire NekRS domain is output
with the usual field file format used by standalone NekRS calculations.

!listing /tutorials/pebble_1/nek.i
  start=Executioner
  end=Postprocessors

Finally, several postprocessors are included. A postprocessor named `flux_integral`
is added automatically by [NekRSProblem](/problems/NekRSProblem.md) to receive the value of the heat flux
integral from MOOSE for internal normalization in NekRS. It is as if the following is added
to the input file:

!listing
[Postprocessors]
  [flux_integral]
    type = Receiver
  []
[]

but the addition is automated for you.
The other three postprocessors are all Cardinal-specific postprocessors that perform
integrals and global min/max calculations over the NekRS domain for diagnostic purposes.
Here, the [NekHeatFluxIntegral](/postprocessors/NekHeatFluxIntegral.md)
postprocessor computes $-k\nabla T\cdot\hat{n}$ over a boundary in the NekRS mesh. This
value should approximately match the imposed heat flux, `flux_integral`, though perfect
agreement is not to be expected since flux boundary conditions are only weakly imposed
in the spectral element method. The
[NekVolumeExtremeValue](/postprocessors/NekVolumeExtremeValue.md) postprocessors then compute the maximum
and minimum temperatures throughout the entire NekRS domain (i.e. not only on the [!ac](CHT)
coupling surfaces).

!listing /tutorials/pebble_1/nek.i
  start=Postprocessors

You will likely notice that many of the almost-always-included MOOSE blocks are absent
from the `nek.i` input file - for instance, there are no nonlinear or auxiliary variables
in the input file.
The [NekRSProblem](/problems/NekRSProblem.md) class assists in input file setup by declaring many of these coupling fields
automatically. For this example, two auxiliary variables named `temp` and `avg_flux` are
added automatically, as if the following were included in the input file:

!listing
[AuxVariables]
  [avg_flux]
  []
  [temp]
  []
[]

These variables receive incoming and outgoing transfers to/from NekRS; the order is set
to match the order of the [NekRSMesh](/mesh/NekRSMesh.md).
You will see both `temp` and `avg_flux` referred to in the solid input file `[Transfers]` block,
in addition to the `flux_integral` [Receiver](https://mooseframework.inl.gov/source/postprocessors/Receiver.html)
postprocessor that receives the integrated heat flux for normalization.

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

A detailed description of all of the available parameters, settings, and use cases
for these input files is available on the
[NekRS documentation website](https://nekrsdoc.readthedocs.io/en/latest/input_files.html).
Because this is a Cardinal tutorial, only
the aspects of NekRS required to understand the present case will be covered. First,
begin with the `fluid.par` file.

!listing /tutorials/pebble_1/fluid.par

Boundaries 1, 2, and 7 are flux boundaries
(these boundaries will receive a heat flux from MOOSE), boundaries 3, 4, and 8 are
insulated, boundary 5 is a specified temperature, and boundary 6 is a zero-gradient
outlet. The actual assignment of values for these boundary conditions is then
performed in the `fluid.oudf` file. The `fluid.oudf` file contains [!ac](OCCA) kernels that
will be run on a [!ac](GPU) (if present). If no [!ac](GPU) is present,
these kernels are simply run with MPI.
Because this case does not have any user-defined
source terms in NekRS, these [!ac](OCCA) kernels are only used to apply boundary conditions.

!listing /tutorials/pebble_1/fluid.oudf language=cpp

The names of these functions correspond to the boundary conditions that were applied
in the `.par` file - only the user-defined temperature and flux boundaries require user
input in the `.oudf` file. For each function, the `bcData` object contains all information
about the current boundary that is "calling" the function; `bc->id` is the boundary ID,
`bc->s` is the scalar (temperature) solution at the present [!ac](GLL) point, and
`bc->flux` is the flux (of temperature) at the present [!ac](GLL) point. The
`bc->usrwrk` array is a scratch space to which the heat flux values coming from MOOSE are
written. These OCCA functions then get called directly within NekRS.

Finally, the `fluid.udf` file contains user-defined C++ functions through
which other interactions with the NekRS solution are performed. Here, the `UDF_Setup` function
is called once at the very start of the NekRS simulation, and it is here that initial
conditions are applied.

!listing /tutorials/pebble_1/fluid.udf language=cpp

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

which will run with 48 MPI ranks. Both MOOSE and NekRS will be run with 48 processes.
When you run this file, Cardinal will
print out a table summarizing all of the non-dimensional scales that are used to re-scale
the non-dimensional NekRS solution when it gets mapped to MOOSE. This can be used to quickly
see the various scales, and understand how time is dimensionalized with the $t_{ref}$.

```
 ------------------------------------------------------------------------------------------------
 |   Time    |  Length   | Velocity  | Temperature | d(Temperature) |  Density  | Specific Heat |
 ------------------------------------------------------------------------------------------------
 | 1.043e-01 | 6.000e-03 | 5.750e-02 | 9.231e+02   | 1.000e+01      | 1.962e+03 | 2.416e+03     |
 ------------------------------------------------------------------------------------------------
```

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
constructed by [NekRSMesh](/mesh/NekRSMesh.md).

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

The solid phase is again solved with the MOOSE heat conduction module; the input file
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
inlet and outlet mass flowrates. Like the [NekVolumeExtremeValue](/postprocessors/NekVolumeExtremeValue.md)
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

Because NekRS is run as a sub-application to MOOSE, the `stopAt` and `numSteps`
fields are actually ignored, so that the steady state tolerance in the MOOSE main
application dictates when a simulation terminates. Because the purpose of this

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
The `velocityDirichletConditions` function applies Dirichlet
conditions to velocity, where `bc->u` is the $x$-component of velocity,
`bc->v` is the $y$-component of velocity, and `bc->z` is the $z$-component of velocity.
In this function, the kernel variable `Vz` was defined in the `fluid.udf` file.
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

!bibtex bibliography
