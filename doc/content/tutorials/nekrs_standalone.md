# Running NekRS as a Standalone Application

In this tutorial, you will learn how to:

- Run NekRS as a standalone application completely separate from Cardinal
- Run a thinly-wrapped NekRS simulation without any physics coupling, to leverage
  Cardinal's postprocessing and I/O features

## Standalone Simulations
  id=standalone

As part of Cardinal's build process, the `nekrs` executable used to run standalone
NekRS cases is compiled and placed in the `$NEKRS_HOME/bin` directory. This directory
also contains other scripts used to simplify the use of the `nekrs` executable.
To use these scripts to run standalone NekRS cases,
we recommend adding this location to your path:

```
export PATH=$NEKRS_HOME/bin:$PATH
```

Then, you can run any standalone NekRS case simply by having built Cardinal -
no need to separately build and compile NekRS. For instance, try running the
`ethier` example that ships with NekRS:

```
cd contrib/nekRS/examples/ethier
nrsmpi ethier 4
```

And that's it! No need to separately compile NekRS.

## Thinly-Wrapped Simulations

To access this tutorial:

```
cd cardinal/tutorials/standalone
```

To contrast with the previous example, you can achieve the same "standalone"
calculations via Cardinal, which you might be interested in to leverage Cardinal's
postprocessing and data I/O features. Some useful features include:

- Query the solution, evaluate heat balances and pressure drops,
  or evaluate solution convergence
- Providing one-way coupling to other MOOSE applications, such as for
  transporting scalars based on NekRS's velocity solution or for projecting
  NekRS turbulent viscosity closure terms onto another MOOSE application's mesh
- Project the NekRS solution onto other discretization schemes,
  such as a subchannel discretization, or onto other MOOSE applications, such as
  for providing closures
- Automatically convert nondimensional NekRS solutions into dimensional form
- Because the MOOSE framework supports many different [output formats](Outputs/index.md), obtain a representation of the NekRS solution in Exodus, VTK, CSV, and other formats.

Instead of running a NekRS input
with the `nekrs` executable, you can instead
create a "thin" wrapper input file that runs NekRS as a MOOSE
application (but allowing usage of the postprocessing and data I/O features of Cardinal).
For wrapped applications, NekRS will continue to write its own field file output
during the simulation as specified by settings in the `.par` file.

To run NekRS via MOOSE, without any physics coupling,
Cardinal simply replaces calls to MOOSE solve methods with NekRS solve methods available
through an [!ac](API). There are no data transfers to/from NekRS. A
thinly-wrapped simulation uses:

1. [NekRSMesh](NekRSMesh.md): create a "mirror" of the NekRS mesh, which can *optionally* be used to interpolate
   a high-order NekRS solution into a lower-order mesh (in any MOOSE-supported format).
2. [NekRSProblem](NekRSProblem.md): allow MOOSE to run NekRS

For this tutorial, we will use the `turbPipe` example that ships with the NekRS repository
as an example case. This case models
turbulent flow in a cylindrical pipe (cast in non-dimensional form). The domain consists
of a pipe of diameter 1 and length 20 with flow in the $+z$ direction. The
NekRS mesh is shown in [nek_mesh]. Note that the visualization of NekRS's
mesh in Paraview draws lines connecting the [!ac](GLL) points, and *not* the actual
element edges.

!media turb_pipe_mesh.png
  id=nek_mesh
  caption=NekRS mesh, with lines connecting [!ac](GLL) points and sideset IDs.
  style=width:60%;margin-left:auto;margin-right:auto;halign:center

The NekRS input files are exactly the same that would be used to run this model
as a standalone case. These input files include:

- `turbPipe.re2`: NekRS mesh
- `turbPipe.par`: High-level settings for the solver, boundary condition mappings
   to sidesets, and the equations to solve
- `turbPipe.udf`: User-defined C++ functions for on-line postprocessing and model setup
- `turbPipe.oudf`: User-defined [!ac](OCCA) kernels for boundary conditions and source terms

This particular input also uses an optional `turbPipe.usr` file for setting up
other parts of the model using the Nek5000 backend. We refer a discussion
of these files to the [NekRS documentation website](https://nekrsdoc.readthedocs.io/en/latest/input_files.html).

Instead of running this input directly with the NekRS scripts like we did
in [#standalone], we instead wrap the NekRS simulation as a MOOSE application.
The Cardinal input file is shown below; this is not the simplest file that we
*need* to run NekRS, but we add extra features to be described shortly.

!listing /tutorials/standalone/nek.i

The essential blocks in the input file are:

- `Mesh`: creates a lower-order mirror of the NekRS mesh
- `Problem`: replaces MOOSE finite element solves with NekRS solves. [NekFieldVariable](NekFieldVariable.md) objects are added in order to read from the NekRS internal solution fields and write onto the [NekRSMesh](NekRSMesh.md) for viewing.
- `Executioner`: controls the time stepping according to the settings in the NekRS input files
- `Outputs`: outputs any results that have been projected onto the [NekRSMesh](NekRSMesh.md) to the specified format.

This input file is run with:

```
mpiexec -np 4 cardinal-opt -i nek.i
```

which will run with 4 MPI ranks. This will create a number of output files:

- `nek_out.e` shows the NekRS solution mapped to a MOOSE mesh
- `nek_out_sub0.` shows the result of a postprocessing operation, mapped to a
  different MOOSE mesh
- `nek_out.csv` shows the CSV postprocessor values
- `turbPipe0.f<n>` are the NekRS output files, where `<n>` is an integer representing output step index in NekRS

When running this tutorial, the NekRS output file is the `nek_out.e` file,
while the output of the sub-application is the `nek_out_sub0.e` file.

Now that you know how to run, let's describe
the rest of the contents in the `nek.i` input file.
This file adds a few additional postprocessing operations to compute:

- pressure drop, computed by subtracting the inlet average pressure from the outlet
  average pressure with two [NekSideAverage](NekSideAverage.md)
  postprocessors and the [DifferencePostprocessor](DifferencePostprocessor.md)
- mass flowrate, computed with a [NekMassFluxWeightedSideIntegral](NekMassFluxWeightedSideIntegral.md)
  postprocessor

!listing /tutorials/standalone/nek.i
  block=Postprocessors

This will print to the screen for each time step the values of these postprocessors,
which can be useful for evaluating solution progression:

```
+----------------+----------------+----------------+
| time           | dP             | mdot           |
+----------------+----------------+----------------+
|   0.000000e+00 |   0.000000e+00 |   0.000000e+00 |
|   6.000000e-03 |  -3.334281e+02 |  -7.854005e-01 |
|   1.200000e-02 |   1.637766e+02 |  -7.853985e-01 |
|   1.800000e-02 |   1.540674e+00 |  -7.853985e-01 |
+----------------+----------------+----------------+
```

By setting `csv = true` in the output block, we will also write these postprocessors
into a CSV format, which is convenient for script-based postprocessing operations:

```
time,dP,mdot
0,0,0
0.006,-333.42814336273,-0.78540045786239
0.012,163.77659967808,-0.78539846245391
0.018,1.540674311383,-0.78539846245391
```

By using [NekFieldVariable](NekFieldVariable.md) objects in the `[FieldTransfers]` block,
we write the NekRS solution for pressure and velocity (which for this example has $(7+1)^3$ degrees of
freedom per element, since `polynomialOrder = 7` in `turbPipe.par`)
onto second-order Lagrange auxiliary variables
named `P` and `velocity_x`, `velocity_y`, and `velocity_z`. You can then apply *any* MOOSE object to those
variables, such as postprocessors, userobjects, auxiliary kernels, and so on.
You can also transfer these variables to another MOOSE application
if you want to couple NekRS to MOOSE *without feedback* - such as for using
Nek's velocity to transport a passive scalar in another MOOSE application.

The axial velocity computed by NekRS, as well as the velocity interpolated onto
the mesh mirror, are shown in [nek_vels].

!media nek_velocity_turbpipe.png
  id=nek_vels
  caption=NekRS computed axial velocity (top) and the velocity interpolated onto the [NekRSMesh](NekRSMesh.md) mirror (bottom)
  style=width:60%;margin-left:auto;margin-right:auto;halign:center

We can also apply several userobjects *directly* to the NekRS solution for a
number of postprocessing operations. Below, we perform a volume average
of $V_z$ in 12 radial bins discretized into 20 axial layers.

!listing /tutorials/standalone/nek.i
  block=UserObjects

!alert note
In Cardinal, and Postprocessors or UserObjects which begin with `Nek` in their name
are not performing operations on the lower-order mapping
of the NekRS solution - they are *directly* doing integrals/averages/etc. on
the [!ac](GLL) points.

If we want to view the output of this averaging on the
[NekRSMesh](NekRSMesh.md), we could visualize it with a
[SpatialUserObjectAux](SpatialUserObjectAux.md).

!listing /tutorials/standalone/nek.i
  start=AuxVariables
  end=MultiApps

The result of the volume averaging operation is shown in [avg1].
Because the NekRS mesh elements don't fall nicely into the specified bins,
we actually can only see the bin averages that the mesh mirror elements "hit"
(according to their centroid). This is obviously non-ideal because the underlying
form of the NekRS mesh is distorting the *visualization* of the volume average
(even though the NekRS mesh element layout doesn't affect the actual averaging
and the userobject stores the values of all 12 radial bins, even if they can't
be seen).

!media vol_avgs_master.png
  id=avg1
  caption=Representation of the `volume_averages` binned averaging on the NekRS mesh mirror
  style=width:65%;margin-left:auto;margin-right:auto;halign:center

Instead, we can
leverage MOOSE's [MultiApp](MultiApps/index.md)
system to transfer the user object to a sub-application with a different mesh
than what is used in NekRS. Then we can visualize the averaging operation
perfectly without concern for the fact that the NekRS mesh doesn't have elements
that fall nicely into the 12 radial bins. To do this,
we create a sub-application with mesh elements exactly matching
the user object binning and turn the solve off by setting `solve = false`, so that
this input file only serves to receive data onto a different mesh.

!listing /tutorials/standalone/sub.i

Then we transfer the `volume_averages` user object to the sub-application.

!listing /tutorials/standalone/nek.i
  start=MultiApps
  end=Executioner

The user object received on the sub-application is shown in [avg2],
which exactly represents the 12 radial averaging bins.

!media vol_avgs_sub.png
  id=avg2
  caption=Representation of the `volume_averages` binned exactly as computed by user object
  style=width:65%;margin-left:auto;margin-right:auto;halign:center

A few examples of other postprocessors that may be of use to NekRS
simulations include:

- [ElementL2Error](ElementL2Error.md),
  which computes the L$^2$ norm of a variable relative to a provided
  function, useful for solution verification
- [FindValueOnLine](FindValueOnLine.md),
  which finds the point at which a specified value of a variable occurs,
  which might be used for evaluating a boundary layer thickness
- [LinearCombinationPostprocessor](LinearCombinationPostprocessor.md),
  which can be used to combine postprocessors together in a
  general expression $a_0p_0+a_1p_1+\cdots+b$, where $a_i$ are coefficients,
  $p_i$ are postprocessors, and $b$ is a constant additive factor. This can be used
  to compute the temperature *rise* in a domain by subtracting a postprocessor
  that computes the inlet temperature from a postprocessor that computes the
  outlet temperature.
- [PercentChangePostprocessor](PercentChangePostprocessor.md) which computes the percent
  change between two successive time steps for assessing convergence.
- [TimeExtremeValue](TimeExtremeValue.md),
  which provides the maximum/minimum value of a variable over the course of
  an entire simulation, such as for evaluating peak stress in an
  oscillating system

Please consult the [MOOSE documentation](https://mooseframework.inl.gov/source/index.html)
for a full list of available postprocessors.
