# Conjugate Heat Transfer for Flow Over a Pebble

In this tutorial, you will learn how to:

- Couple NekRS with MOOSE for [!ac](CHT) for laminar flow over a single heated pebble

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
- 3: pebble surface
- 4: side walls

!media pebble_1.png
  id=pebble_1
  caption=NekRS flow domain. Sidesets in the fluid domain are colored on the right.
  style=width:30%;margin-left:auto;margin-right:auto;halign:center

NekRS shall solve for laminar flow over this pebble. Details on the problem
specifications are given in [table1]. The inlet velocity is specified such that
the Reynolds number is $Re=50$.

The MOOSE heat conduction module shall be used to solve for the solid temperature.
NekRS and MOOSE will be coupled through boundary conditions on the pebble surface;
NekRS shall compute a pebble surface temperature to be applied as a Dirichlet condition
to MOOSE, while MOOSE shall compute a pebble surface heat flux to be applied
as a Neumann condition in NekRS.
The pebble power is selected to
give a pebble power of approximately 223 W (giving a bulk fluid temperature rise
of 50 K).

!table id=table1 caption=Geometric and operating conditions for the single-pebble flow
| Parameter | Value |
| :- | :- |
| Pebble diameter | 0.03 m |
| Domain height | 0.4306 m |
| Inlet flow area | 0.00064009 m |
| Inlet velocity | 0.001666 m/s |
| Fluid viscosity | 1e-3 Pa-s |
| Fluid density | 1000 kg/m$^3$ |
| Fluid thermal conductivity | 0.6 W/m/K |
| Fluid volumetric specific heat | 4186 J/kg/K |
| Pebble power density | 15774023 W/m$^3$ |

## CHT Coupling

In this example, the overall calculation workflow is as follows:

1. Run MOOSE heat conduction with a given surface temperature distribution from NekRS.
2. Send heat flux to NekRS as a boundary condition.
3. Run NekRS with a given surface heat flux distribution from MOOSE.
4. Send surface temperature to MOOSE as a boundary condition.

The above sequence is repeated until convergence.

### Fluid Input Files
  id=fluid_model

NekRS is used to solve the [incompressible Navier-Stokes equations](theory/ins.md).
We already created the
input files for NekRS in the [NekRS introduction tutorial](tutorials/nek_intro.md).
If you have not reviewed this tutorial, please be sure to do so before proceeding.
We will only describe the aspects of our NekRS setup which *differ* from this
previous tutorial.

In order to keep our files separate, we will use `pebble_cht` as our casename here.

#### .par File

Our `.par` file is the same as the [standalone NekRS tutorial](tutorials/nek_intro.md)
except that we will point to the same mesh and `.udf` file from before by specifying
the `file` and `udf` parameters.

!listing /tutorials/pebble_1/pebble_cht.par

### .oudf File

For conjugate heat transfer coupling to MOOSE, we only need to change one line
in the NekRS `.oudf` file to apply a heat flux boundary condition
from MOOSE. In the `scalarNeumannConditions` function, we simply
need to set

```
bc->flux = bc->usrwrk[bc->idM];
```

We will explain in more detail shortly what this line means. Other than this single
change, you are ready to use the same NekRS files to couple to MOOSE.

#### MOOSE Wrapping

Aside from NekRS's input files, the wrapping of NekRS as a MOOSE
application is specified in a "thin" MOOSE-type input file (which we named `nek.i`
for this example).

A mirror of the NekRS mesh
is constructed using the [NekRSMesh](/mesh/NekRSMesh.md). The
`boundary` parameter indicates the boundaries through which NekRS is coupled
via conjugate heat transfer to MOOSE.

!listing /tutorials/pebble_1/nek.i
  block=Mesh

!alert note
Note that `pebble.re2` does not appear anywhere in `nek.i` - the `pebble.re2` file is
a mesh used directly by NekRS, while [NekRSMesh](/mesh/NekRSMesh.md) is a mirror of the boundaries in `pebble.re2`
through which boundary coupling with MOOSE will be performed.

Next, the [Problem](https://mooseframework.inl.gov/syntax/Problem/index.html)
 block describes all objects necessary for the actual physics solve.
To replace MOOSE finite element calculations with NekRS
spectral element calculations, the [NekRSProblem](problems/NekRSProblem.md) class is used.
The `casename` is used to supply the file name prefix for
the NekRS input files.

!listing /tutorials/pebble_1/nek.i
  start=Problem
  end=Executioner

Next, a [Transient](https://mooseframework.inl.gov/source/executioners/Transient.html) executioner
is specified. This is the same executioner used for most transient MOOSE simulations, except now a
different time stepper is used - [NekTimeStepper](/timesteppers/NekTimeStepper.md).
This time stepper simply
reads the time step specified in NekRS's `.par` file.
Except for synchronziation points
with the MOOSE application(s) to which NekRS is coupled, NekRS controls all of its own
time stepping. Note that in this example, NekRS will be run as a sub-application of
the heat conduction solve. In this case, the `numSteps` settings
in the `.par` file
are actually ignored, so that the main MOOSE application controls when
the simulation terminates.

!listing /tutorials/pebble_1/nek.i
  block=Executioner

An Exodus II output format is specified.
It is important to note that this output file only outputs the NekRS solution fields that have
been interpolated onto the mesh mirror; the solution over the entire NekRS domain is output
with the usual field file format used by standalone NekRS calculations.

!listing /tutorials/pebble_1/nek.i
  block=Outputs

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
A postprocessor named `flux_integral`
is also added automatically to receive the value of the heat flux
integral from MOOSE for internal normalization in NekRS. It is as if the following is added
to the input file:

!listing
[Postprocessors]
  [flux_integral]
    type = Receiver
  []
[]

You will see `temp`, `avg_flux`, and `flux_integral` referred to in the solid input file `[Transfers]` block,

### Solid Input Files

The MOOSE heat conduction module is used to solve for
[energy conservation in the solid](theory/heat_eqn.md).

First, a local variable, `pebble_diameter` is used to conveniently be able to
repeat data throughout a MOOSE input file. Then, anywhere in the input file that
we want to refer to the value `0.03`, we can use bash-type syntax like `${pebble_diameter}`.
Note that these names are completely arbitrary, and simply help to streamline setup for
complex MOOSE input files.

!alert tip
You can use similar syntax to do math inside a MOOSE input file. For example, to populate
a location in the input file with half the value held by the file-local variable
`pebble_diameter`, you would write `${fparse 0.5 * pebble_diameter}`.

The mesh is generated using MOOSE's [SphereMeshGenerator](https://mooseframework.inl.gov/source/meshgenerators/SphereMeshGenerator.html).
You can either generate this mesh "online" as part of the simulation setup, or
we can create it as a separate activity and then load it (just as you can load any Exodus
mesh into a MOOSE simulation). We will do the former here, but still show you
how you can generate a mesh.

!listing /tutorials/pebble_1/solid.i
  block=Mesh

We can run this file in "mesh-only mode" (which will skip all of the solves) to generate an Exodus
mesh with

```
cardinal-opt -i solid.i --mesh-only
```

which will create a file name `solid_in.e` which contains the mesh. Note
that you do not *need* to do this! When we run our simulation later, the mesh
will already be visible in the output file. This is strictly showing you how you would
generate *just* the mesh from a MOOSE input file.

If we open `solid_in.e` in Paraview, we can see the mesh as shown in [one_pebble_mesh].
The surface of the pebble is sideset 0.

!media one_pebble_mesh.png
  id=one_pebble_mesh
  caption=Mesh used for the solid heat conduction.
  style=width:30%;margin-left:auto;margin-right:auto;halign:center

The heat conduction module will solve for temperature, which is defined as a nonlinear
variable.

!listing /tutorials/pebble_1/solid.i
  block=Variables

Next, the governing equation solved by MOOSE is specified with the `Kernels` block as the
[HeatConduction](https://mooseframework.inl.gov/source/kernels/HeatConduction.html)
 kernel plus the
[BodyForce](https://mooseframework.inl.gov/source/kernels/BodyForce.html) kernel, or
On the fluid-solid interface,
a [MatchedValueBC](https://mooseframework.inl.gov/source/bcs/MatchedValueBC.html)
 applies the value of a variable named `nek_temp` (discussed soon) as a Dirichlet condition.
The [HeatConduction](https://mooseframework.inl.gov/source/kernels/HeatConduction.html)
 kernel requires a material property for the thermal conductivity.

\begin{equation}
-\nabla\cdot(k\nabla T)-\dot{q}=0$
\end{equation}

!listing /tutorials/pebble_1/solid.i
  start=Kernels
  end=Executioner

[AuxVariables](https://mooseframework.inl.gov/source/variables/AuxVariable.html)
are used to represent field data which passes between different applications. Here,
we need to define an auxiliary variable to represent the field data for wall temperature
(`nek_temp`, to be *received* from NekRS) and the wall heat flux (`flux`, to be *sent*
to NekRS).
A [DiffusionFluxAux](https://mooseframework.inl.gov/source/auxkernels/DiffusionFluxAux.html) auxiliary kernel is specified
to compute the flux on the `fluid_solid_interface` boundary. The `flux` variable must be
a monomial field due to the nature of how MOOSE computes material properties.

!listing /tutorials/pebble_1/solid.i
  start=AuxVariables
  end=Postprocessors

For the very first
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
  end=AuxVariables

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
  block=Postprocessors

Next, the solution methodology is specified. Although the solid phase only
includes time-independent kernels, the heat conduction is run as a transient because NekRS
ultimately must be run as a transient (NekRS lacks a steady solver). We choose
to omit the time derivative in the solid energy equation because we will reach
the converged steady state faster than if the solve had to also ramp up the solid
temperature from the initial condition. We will terminate the coupled solve once
the relative change in the solid temperature is smaller than the
`steady_state_tolerance`.

!listing /tutorials/pebble_1/solid.i
  start=Executioner
  end=MultiApps

### Execution and Postprocessing
  id=ep

To run the conjugate heat transfer model:

```
mpiexec -np 4 cardinal-opt -i solid.i
```

which will run with 4 MPI ranks. Both MOOSE and NekRS will be run with 4 processes.
When you run Cardinal, a table will be printed out that shows all of the quantities
in `usrwrk` (which is where MOOSE places its heat flux, and is what you used in the
`.oudf` file to apply this boundary condition). This example *only* exchanges heat flux
from MOOSE to NekRS, so the rest of the quantities in this space (called "scratch space")
are actually unused. But it is in this table where you can find out what the "slots" in
the scratch space represent from MOOSE if you are unsure.

```
  ===================>     MAPPING FROM MOOSE TO NEKRS      <===================

           Slice:  entry in NekRS scratch space
        Quantity:  physical meaning or name of data in this slice
   How to Access:  C++ code to use in NekRS files; for the .udf instructions,
                   'n' indicates a loop variable over GLL points

 ------------------------------------------------------------------------------------------------
 | Quantity |           How to Access (.oudf)           |         How to Access (.udf)          |
 ------------------------------------------------------------------------------------------------
 | flux     | bc->usrwrk[0 * bc->fieldOffset + bc->idM] | nrs->usrwrk[0 * nrs->fieldOffset + n] |
 | unused   | bc->usrwrk[1 * bc->fieldOffset + bc->idM] | nrs->usrwrk[1 * nrs->fieldOffset + n] |
 | unused   | bc->usrwrk[2 * bc->fieldOffset + bc->idM] | nrs->usrwrk[2 * nrs->fieldOffset + n] |
 | unused   | bc->usrwrk[3 * bc->fieldOffset + bc->idM] | nrs->usrwrk[3 * nrs->fieldOffset + n] |
 | unused   | bc->usrwrk[4 * bc->fieldOffset + bc->idM] | nrs->usrwrk[4 * nrs->fieldOffset + n] |
 | unused   | bc->usrwrk[5 * bc->fieldOffset + bc->idM] | nrs->usrwrk[5 * nrs->fieldOffset + n] |
 | unused   | bc->usrwrk[6 * bc->fieldOffset + bc->idM] | nrs->usrwrk[6 * nrs->fieldOffset + n] |
 ------------------------------------------------------------------------------------------------
```

When the simulation has completed, you will have created a number of different output files:

- `pebble_cht0.f<n>`, the NekRS output files
- `solid_out.e`, an Exodus II output file with the solid mesh and solution
- `solid_out_nek0.e`, an Exodus II output file with the fluid mirror mesh
  and data that was ultimately transferred in/out of NekRS

First, let's take a look at the two meshes *together*. [fluid_and_solid] shows a slice
through the NekRS mesh (with quadrature points shown) and the solid pebble mesh
(in yellow). Cardinal does *not* require conformality between the meshes - by using
MOOSE's nearest node transfer, we don't even require overlap between the meshes.
Of course there will be some interpolation error when the meshes are not exactly
the same, but you can always conduct a mesh refinement study until you are happy with
the numerical error in the mapping.

!media fluid_and_solid.png
  id=fluid_and_solid
  caption=NekRS mesh (gray) and solid mesh (yellow)
  style=width:30%;margin-left:auto;margin-right:auto;halign:center

[temperature_cht5] shows the fluid temperature (NekRS) and solid temperature (MOOSE).
The conjugate heat transfer coupling between the two phases is evident from the continuity
in temperature on the pebble surface.

!media temperature_cht5.png
  id=temperature_cht5
  caption=Fluid temperature (NekRS) and solid temperature (MOOSE)
  style=width:50%;margin-left:auto;margin-right:auto;halign:center
