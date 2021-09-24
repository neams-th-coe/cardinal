# Tutorial 1A: Running NekRS as a Standalone Application

In this tutorial, you will learn how to:

- Run NekRS as a standalone application completely separate from Cardinal
- Run a thinly-wrapped NekRS simulation without any physics coupling

This tutorial makes use of the following major Cardinal classes:

- [NekRSMesh](/mesh/NekRSMesh.md)
- [NekTimeStepper](/timesteppers/NekTimeStepper.md)
- [NekRSStandaloneProblem](/problems/NekRSStandaloneProblem.md)

We recommend quickly reading this documentation before proceeding
with this tutorial.
!alert-end!

This tutorial provides a description for how to use Cardinal to run both
entirely-standalone NekRS cases as well as thinly-wrapped NekRS inputs to
leverage Cardinal's postprocessing and I/O features. Each of these capabilities
is described separately.

## Standalone Simulations
  id=standalone

As part of Cardinal's build process, the `nekrs` executable used to run standalone
NekRS cases is compiled and placed in the `$NEKRS_HOME/bin` directory. In this directory
are also all the usual scripts used to simplify the use of the `nekrs` executable.
To use these scripts to run standalone NekRS cases,
we recommend adding this location to your path:

```
$ export PATH=$NEKRS_HOME/bin:$PATH
```

Then, you can run any standalone NekRS case simply by having built Cardinal -
no need to separately build and compile NekRS. For instance, try running the
`ethier` example that ships with NekRS:

```
$ cd contrib/nekRS/examples/ethier
$ nrsmpi ethier 4
```

## Thinly-Wrapped Simulations

Cardinal offers many convenient features for running [!ac](CFD) simulations that
are not available directly in NekRS. Notable features include:

- Postprocessing operations to evaluate max/mins, area/volume integrals and averages,
  and mass flux-weighted side integrals of various quantities in the [!ac](CFD) solution
- Extracting the NekRS solution into any output format supported by MOOSE (such as
  Exodus and VTK - see the full list of formats supported by MOOSE
  [here](https://mooseframework.inl.gov/syntax/Outputs/index.html)).

If your main intention is *not* to couple NekRS to MOOSE, then Cardinal can be used
simply to provide extra postprocessing and data I/O features that would be
time-consuming to implement from scratch in NekRS. Instead of running a NekRS input
with the `nekrs` executable, you can instead
create a "thin" wrapper input file that essentially just runs NekRS as a MOOSE
application (but allowing usage of the postprocessing and data I/O features of Cardinal).

For a thin wrapping of NekRS as a MOOSE application, without any physics coupling,
Cardinal simply replaces calls to MOOSE solve methods with NekRS solve methods available
through an [!ac](API) (but no data transfers occur between NekRS and MOOSE). A
thinly-wrapped simulation uses the [NekRSStandaloneProblem](/problems/NekRSStandaloneProblem.md)
and [NekRSMesh](/mesh/NekRSMesh.md) classes to:

1. Create a "mirror" of the NekRS mesh, which can *optionally* be used to interpolate
   a high-order NekRS solution into a lower-order mesh (which can be in any MOOSE-supported format).
2. Provide access to NekRS internals such that a wide variety of Cardinal
   postprocessors and user objects can be used to analyze a NekRS simulation.

All input files for this example are in the `tutorials/standalone` directory. These input
files are the same as the `turbPipe` example in the NekRS directory, which models
turbulent flow in a cylindrical pipe (cast in non-dimensional form). The domain consists
of a pipe of diameter 1 and length 20 with flow in the positive $z$ direction. The
NekRS mesh with sidesets is shown in [nek_mesh]. Note that the visualization of NekRS's
mesh in Paraview draws lines connecting the [!ac](GLL) points, and *not* the actual
element edges.

!media turb_pipe_mesh.png
  id=nek_mesh
  caption=NekRS mesh, with lines connecting [!ac](GLL) points and sideset IDs listed in Courier font.
  style=width:60%;margin-left:auto;margin-right:auto;halign:center

The NekRS input files are exactly the same that would be used to run this model
as a standalone case. These input files include:

- `turbPipe.re2`: NekRS mesh
- `turbPipe.par`: High-level settings for the solver, boundary condition mappings
   to sidesets, and the equations to solve
- `turbPipe.udf`: User-defined C++ functions for on-line postprocessing and model setup
- `turbPipe.oudf`: User-defined [!ac](OCCA) kernels for boundary conditions and source terms

This particular input also uses an optional `turbPipe.usr` file for setting up
other parts of the model using the Nek5000 backend. Because these NekRS input files
require no modification to be wrapped in a MOOSE simulation, we refer a discussion
of these files to the [NekRS documentation website](https://nekrsdoc.readthedocs.io/en/latest/input_files.html).
In [Tutorial 2A](cht1.md), we will revisit these files and describe the (very minor)
modifications required to *couple* NekRS to MOOSE.

Instead of running this input directly with the NekRS scripts like we did
in [#standalone], we instead wrap the NekRS simulation as a MOOSE application.
The thin-wrapped input file is shown below.

!listing /tutorials/standalone/nek.i

The essential blocks in the input file are the `Mesh` (which creates a lower-order mirror
of the NekRS mesh), `Problem` (which replaces MOOSE finite element solves with
NekRS solves), `Executioner` (which controls the time stepping according to the settings
in the NekRS input files), and `Outputs` (which outputs any results that have been
projected onto the [NekRSMesh](/mesh/NekRSMesh.md) to the specified format.

In this tutorial, we will add a few postprocessing operations to illustrate
the utility of wrapping NekRS simulations in Cardinal. First we add several
postprocessors to compute the pressure drop and the mass flowrate. The pressure
drop is computed by subtracting the inlet average pressure from the outlet
average pressure with two [NekSideAverage](/postprocessors/NekSideAverage.md)
postprocessors. The mass flowrate is computed based on the inlet boundary
with a [NekMassFluxWeightedSideIntegral](/postprocessors/NekMassFluxWeightedSideIntegral.md)
postprocessor.

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

By setting `output = 'pressure velocity'` for [NekRSStandaloneProblem](/problems/NekRSStandaloneProblem.md),
we interpolate the NekRS solution (which for this example has $(7+1)^3$ degrees of
freedome per element, since `polynomialOrder = 7` in the `turbPipe.par` file)
onto a second-order version of the same mesh by creating
[MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
named `P` and `velocity`. You can then apply *any* MOOSE object to those
variables, such as postprocessors, userobjects, auxiliary kernels, and so on.
You can also transfer these variables to another MOOSE application
if you want to couple NekRS to MOOSE *without feedback* - such as for using
Nek's velocity to transport a passive scalar in another MOOSE application.

The axial velocity computed by NekRS, as well as the velocity interpolated onto
the mesh mirror, are shown in [nek_vels].

!media nek_velocity_turbpipe.png
  id=nek_vels
  caption=NekRS computed axial velocity (top) and the velocity interpolated onto the [NekRSMesh](/mesh/NekRSMesh.md) mirror (bottom)
  style=width:60%;margin-left:auto;margin-right:auto;halign:center

We can also apply several userobjects directly to the NekRS solution for a
number of postprocessing operations. In this input, we perform a volume average
of the axial velocity component in 12 radial bins discretized into 20 axial layers.

!listing /tutorials/standalone/nek.i
  block=UserObjects

Now, if we wanted to view the output of this averaging on the
[NekRSMesh](/mesh/NekRSMesh.md), we could visualize it with a
[SpatialUserObjectAux](https://mooseframework.inl.gov/source/auxkernels/SpatialUserObjectAux.html).

!listing /tutorials/standalone/nek.i
  begin=AuxVariables
  end=MultiApps

The result of the volume averaging operation is shown in [avg1].
Because the NekRS mesh elements don't fall nicely into the specified bins,
we actually can only see the bin averages that the mesh mirror elements "hit".

!media


Instead, we can
leverage MOOSE's [MultiApp](https://mooseframework.inl.gov/syntax/MultiApps/index.html)
system to transfer the user object to a sub-application with a different mesh
than what is used in NekRS. Then we can visualize the averaging operation
perfectly without concern for the fact that the NekRS mesh doesn't have elements
that fall nicely into the 12 radial bins.

We create a sub-application with a mesh created with elements exactly matching
the user object binning and turn the solve off by setting `solve = false`, so that
this input file only serves to receive data onto a different mesh.

!listing /tutorials/standalone/sub.i


