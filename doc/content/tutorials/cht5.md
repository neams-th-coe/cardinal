# Conjugate Heat Transfer for Flow Over a Pebble

In this tutorial, you will learn how to:

- Couple NekRS with MOOSE for [!ac](CHT) for laminar flow over a single heated pebble
- Solve NekRS in non-dimensional form while MOOSE solves in dimensional form

To access this tutorial,

```
cd cardinal/tutorials/pebble_1
```

!alert! note title=Computing Needs
No special computing needs are required for this tutorial.
!alert-end!

## Geometry and Computational Model

The domain consists of a single pebble within a rectangular box; the pebble
origin is at $(0, 0, 0)$. [pebble_1] shows the fluid portion of the domain (the
pebble is not shown). The sideset numbering in the fluid domain is:

- 1: inlet
- 2: outlet
- 3: lateral walls
- 4: pebble surface

!media pebble_1.png
  id=pebble_1
  caption=Flow domain outside pebble (gray). Sidesets in the fluid domain are colored on the right.
  style=width:30%;margin-left:auto;margin-right:auto;halign:center

NekRS shall solve for laminar flow over this pebble; the Reynolds number is $Re=50$.
The MOOSE heat conduction module shall be used to solve for the solid temperature.
NekRS and MOOSE will be coupled through boundary conditions on the pebble surface;
NekRS shall compute a pebble surface temperature to be applied as a Dirichlet condition
to MOOSE, while MOOSE shall compute a pebble surface heat flux to be applied
as a Neumann condition in NekRS.

## CHT Coupling

### Fluid Input Files
  id=fluid_model

NekRS is used to solve the [incompressible Navier-Stokes equations](theory/ins.md)
in [non-dimensional form](theory/nondimensional_ns.md). We already created the
input files for NekRS in the [NekRS introduction tutorial](tutorials/nek_intro.md).
If you have not reviewed this tutorial, please be sure to do so before proceeding.

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
Note that `pebble.re2` does not appear anywhere in `nek.i` - the `pebble.re2` file is
a mesh used directly by NekRS, while [NekRSMesh](/mesh/NekRSMesh.md) is a mirror of the boundaries in `pebble.re2`
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


The names of these functions correspond to the boundary conditions that were applied
in the `.par` file - only the user-defined temperature and flux boundaries require user
input in the `.oudf` file. For each function, the `bcData` object contains all information
about the current boundary that is "calling" the function; `bc->id` is the boundary ID,
`bc->s` is the scalar (temperature) solution at the present [!ac](GLL) point, and
`bc->flux` is the flux (of temperature) at the present [!ac](GLL) point. The
`bc->usrwrk` array is a scratch space to which the heat flux values coming from MOOSE are
written. These OCCA functions then get called directly within NekRS.

Finally, the `pebble.udf` file contains user-defined C++ functions through
which other interactions with the NekRS solution are performed. Here, the `UDF_Setup` function
is called once at the very start of the NekRS simulation, and it is here that initial
conditions are applied.

!listing /tutorials/pebble_1/pebble.udf language=cpp

The initial condition is applied by looping over all
the [!ac](GLL) points and setting zero to each (recall that this is a non-dimensional
simulation, such that $T^\dagger=0$ corresponds to a dimensional temperature of $T_{ref}$).
Here, `nrs->cds->S` is the array holding the NekRS passive scalar solutions (of which
there is only one for this example).


### Solid Input Files

The MOOSE heat conduction module is used to solve for
[energy conservation in the solid](theory/heat_eqn.md).
The mesh is generated using MOOSE's [SphereMeshGenerator](https://mooseframework.inl.gov/source/meshgenerators/SphereMeshGenerator.html).
You can either generate this mesh "online" as part of the simulation setup, or
we can create it as a separate activity and then load it (just as you can load any Exodus
mesh into a MOOSE simulation). We will do the former here, but still show you
how you can generate a mesh.

The mesh is specified in the `[Mesh]` block of the `solid.i` file.

!listing /tutorials/pebble_1/solid.i
  block=Mesh

We can run this file in "mesh-only mode" to generate an Exodus
mesh with

```
cardinal-opt -i solid.i --mesh-only
```

which will create a file name `solid_in.e` which contains the mesh. Note
that you do not *need* to do this! When we run our simulation later, the mesh
will already be visible in the output file. This is strictly showing you how you would
generate *just* the mesh from a MOOSE input file.

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
  according to the settings in the `pebble.par` file). An extra program is required to visualize
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
drop in the pebble.

Because NekRS is run as a sub-application to MOOSE, the `stopAt` and `numSteps`
fields are actually ignored, so that the steady state tolerance in the MOOSE main
application dictates when a simulation terminates. Because the purpose of this

The `pebble.udf` file is shown below. The `UDF_Setup` function is again used to apply initial
conditions; because temperature is read from the restart file, only initial conditions on
velocity and pressure are required. `nrs->U` is an array storing the three components of
velocity (padded with length `nrs->fieldOffset`), while `nrs->P` is the array storing the
pressure solution.

!listing /tutorials/fhr_reflector/cht/pebble.udf language=cpp

This file also includes the `UDF_LoadKernels` function, which is used to propagate
quantities to variables accessible through [!ac](OCCA) kernels. The `kernelInfo`
object is used to define two variables - `Vz` and `inlet_T` that will be accessible
through the [!ac](GPU) kernels, eliminating some burden on the user if the problem
setup must be changed in multiple locations throughout the NekRS input files.

Finally, the `pebble.oudf` file is shown below. Because the velocity is enabled,
additional boundary condition functions must be specified.
The `velocityDirichletConditions` function applies Dirichlet
conditions to velocity, where `bc->u` is the $x$-component of velocity,
`bc->v` is the $y$-component of velocity, and `bc->z` is the $z$-component of velocity.
In this function, the kernel variable `Vz` was defined in the `pebble.udf` file.
The other boundary conditions -
the Dirichlet temperature conditions and the Neumann heat flux conditions - are the
same as for the steady conduction case.

!listing /tutorials/fhr_reflector/cht/pebble.oudf language=cpp

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
