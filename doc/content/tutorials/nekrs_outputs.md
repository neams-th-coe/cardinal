# Tutorial 5: Postprocessing/Extracting the NekRS solution

In this tutorial, you will learn how to:

- Postprocess a NekRS simulation (both "live" runs as well as from field files)
- Extract the NekRS solution into [MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
  for use with other MOOSE objects

Many of the features covered in this tutorial have already been touched upon
and used in the preceding NekRS tutorials. The purpose of this dedicated tutorial
is to explain more specific use cases or complex combinations of postprocessing/extraction
features. We will use a variety of input cases, and will not focus too much
on the physics setup - just the postprocessing and data extraction features.
For simplicity, many of the examples shown here just deal with thin wrappings
of NekRS via [NekRSStandaloneProblem](/problems/NekRSStandaloneProblem.md) -
however, all features shown here can also be used in a coupled sense via
[NekRSProblem](/problems/NekRSProblem.md).

## Loading a NekRS Time History into Exodus

For applications with "one-way" coupling of NekRS to MOOSE, you may wish
to use a time history of the NekRS solution as a boundary condition/source
term in another MOOSE application without multiphysics coupling on each time
step. For instance, for thermal striping applications, it is often a reasonable
approximation to solve a NekRS CFD simulation as a standalone case, and then
apply a time history of NekRS's wall temperature as a boundary condition to a tensor mechanics solve.
Cardinal allows you to run NekRS wrapped as a MOOSE application and, for each time
step, write the NekRS solution to an Exodus file that can then be loaded to provide
a time history of the CFD solution to another application. An example showing
this usage is shown in the `tutorials/load_from_exodus` directory; we use Cardinal
to run the `ethier` example in NekRS with the `nek.i` input file:

!listing /tutorials/load_from_exodus/nek.i

We incidate that we want to output the NekRS temperature onto a volume mesh mirror
of second order and output to Exodus format. Then, we simply use a
[NekRSStandaloneProblem](/problems/NekRSStandaloneProblem.md)
and [NekTimeStepper](/timesteppers/NekTimeStepper.md) to run the NekRS CFD
calculation through the Cardinal wrapper. You can run this example with

```
$ mpiexec -np 4 cardinal-opt -i nek.i
```

which will create an output file named `nek_out.e`, which contains the time
history of the NekRS temperature solution on the volume mesh mirror.

In order to load the entire time history of the Nek solution, we need a separate
input file that will essentailly act as the surrogate for performing the Nek solution
(by instead loading from the Exodus file dumped earlier). This input file is
the `load_nek.i` file. First, we use the dumped
output file directly as the mesh:

!listing /tutorials/load_from_exodus/load_nek.i
  block=Mesh

Because this run won't actually solve any physics (either with NekRS or with MOOSE),
we next need to turn the solve off.

!listing /tutorials/load_from_exodus/load_nek.i
  block=Problem

Then, to load the solution from the `nek_out.e`, we use a
[SolutionUserObject](https://mooseframework.inl.gov/source/userobject/SolutionUserObject.html).
This user object will read the `temp` variable from the provided `mesh` file
(which we set to our output file from the NekRS run). We omit the `timestep` parameter
that is sometimes provided to the
[SolutionUserObject](https://mooseframework.inl.gov/source/userobject/SolutionUserObject.html)
so that this user object will *interpolate* in time based on the time stepping specified
in the `[Executioner]` block.

This user object only loads the Exodus file into a user object - to then get that
solution into an [AuxVariable](https://mooseframework.inl.gov/syntax/AuxVariables/index.html)
appropriate for transferring to another application or using in other MOOSE objects,
we simply convert the user object to an auxiliary variable using a
[SolutionAux](https://mooseframework.inl.gov/source/auxkernels/SolutionAux.html).
This will load the `temp` variable from the [SolutionUserObject](https://mooseframework.inl.gov/source/userobject/SolutionUserObject.html)
and place it into the new variable we have named `nek_temp`.

!listing /tutorials/load_from_exodus/load_nek.i
  start=AuxVariables

Finally, we "run" this application by specifying a [Transient](https://mooseframework.inl.gov/source/executioners/Transient.html)
executioner. The time stepping scheme we specify here just indicates at which time
step the data in the `nek_out.e` file should be interpolated to. For instance, if you ran
NekRS with a time step of 1e-3 seconds, but only want to couple NekRS's temperature to a
tensor mechanics solve on a resolution of 1e-2 seconds, then simply set the time step size
in this file to `dt = 1e-2`. Finally, we specify a Exodus output in this file, which you can
use to see that the temperature from `nek_out.e` was correctly loaded (`nek_temp` in
`load_nek_out.e` matches `temp` in `nek_out.e`).

## Binned Spatial Postprocessors

Cardinal contains features for postprocessing the NekRS solution in spatial
"bins" using user objects. Available user objects include:

- [NekBinnedSideIntegral](/userobjects/NekBinnedSideIntegral.md):
  compute plane integrals over regions of space
- [NekBinnedSideAverage](/userobjects/NekBinnedSideAverage.md):
  compute plane averages over regions of space
- [NekBinnedVolumeIntegral](/userobjects/NekBinnedVolumeIntegral.md):
  compute volume integrals over regions of space
- [NekBinnedVolumeAverage](/userobjects/NekBinnedVolumeAverage.md):
  compute volume averages over regions of space

These user objects can be used for operations such as:

- Averaging the solution over $\theta$ for axisymmetric geometries
- Extracting homogenized solutions over specified domains, such as for
  feeding volume-averaged quantities to a 3-D Pronghorn model
- Represent the solution in a different discretization form, such as
  a finite volume discretization of a subchannel geometry

An example of averaging over axisymmetric geometries was provided in
[Tutorial 1](tutorials/nekrs_standalone.md). Here, we will demonstrate
averaging the NekRS solution in a pincell geometry according to a subchannel
discretization. The input files for this tutorial are available in
the `tutorials/subchannel` directory.

The model consists of a 7-pin
[!ac](SFR) fuel bundle; the mesh is shown in [bundle]. Our goal is to obtain
subchannel-averaged and gap-averaged temperatures on a mesh that reflects a subchannel discretization.
Note that the mesh does not have elements that
align with the usual subchannel boundaries.

!media sfr_nek_mesh.png
  id=bundle
  caption=NekRS mesh for 7-pin [!ac](SFR) fuel bundle; lines are shown connecting [!ac](GLL) points
  style=width:60%;margin-left:auto;margin-right:auto;halign:center

The NekRS input files use typical settings, for which we refer the reader
to the [NekRS documentation website](https://nekrsdoc.readthedocs.io/en/latest/index.html)
for details. The only setting of note is that for simplicity and ease of illustration, we turn
the actual physics solve off by setting `solver = none` for the velocity and temperature in
the `sfr_7pin.par` file.

!listing /tutorials/subchannel/sfr_7pin.par

We will demonstrate the binned averaging process by setting function initial conditions for
the NekRS solution in the `sfr_7pin.udf` file. Because we turn the solve off, the velocity,
pressure, and temperature that we set in `sfr_7pin.udf` remains fixed for all NekRS
time steps.

!listing /tutorials/subchannel/sfr_7pin.udf language=cpp

For instance, the temperature in the NekRS field files will remain fixed at the
given function initial condition of

\begin{equation}
\label{eq:one}
T=x+y+3z
\end{equation}

We will run this NekRS case with a thin wrapper input file, shown below. Most settings
are conventional and have been described thoroughly in the preceding tutorials.

!listing /tutorials/subchannel/nek.i

Of note are the user objects, multiapps, and transfers. In this file, we will compute
an average of temperature over the volumes of the subchannels and over the gaps between
subchannels. First, we will describe the averages over the volumes of the subchannels.
We will use a user object to
conduct a binned spatial average of the NekRS temperature. We first form these bins
as the product of unique indices for each subchannel with unique indices for each
axial layer. The [HexagonalSubchannelBin](/userobjects/HexagonalSubchannelBin.md)
object assigns a unique ID for each subchannel in a hexagonal fuel bundle, for which
we must provide various geometric parameters describing the fuel bundle. The
[LayeredBin](/userobjects/LayeredBin.md) object assigns a unique ID for equal-size
layers in a given direction; here, we specify 7 axial layers. Finally, we
will compute a volume average of the NekRS temperature with the
[NekBinnedVolumeAverage](/userobjects/NekBinnedVolumeAverage.md) object, which
takes an arbitrary number of individual bin distributions. Because the NekRS mesh
in [bundle] has 6 elements in the axial direction, but we have specified that we
want to average across 7 axial layers, we set `map_space_by_qp = true`. This setting
will essentially apply a delta function of unity in each layer when forming the
integral. Setting `map_space_by_qp = false` instead maps each NekRS element to
a bin according to its centroid (which for this particular example would result in
some of the axial bins not mapping to any elements, since the NekRS mesh is coarser
than the specified number of bins).

For the gap average of temperature, we instead form the bins as the product of unique
indices for each subchannel gap with unique indices for each axial layers.
The [HexagonalSubchannelGapBin](/userobjects/HexagonalSubchannelGapBin.md)
object assigns a unique ID for each subchannel gap ID in a hexagonal fuel bundle. The
[LayeredBin](/userobjects/LayeredBin.md) object then assigns a unique ID for equal-size
layers in the axial direction; for simplicity, we reuse the same axial binning distribution
that we selected for the volume averages. Finally, we compute the planar averages of the
NekRS temperature with the [NekBinnedSideAverage](/userobjects/NekBinnedSideAverage.md)
object, which takes an arbitrary number of individual bin distributions (with the
only requirement being that one and only one of these distribution is a "side" distribution, which
for this example is the `HexagonalSubchannelGapBin`).

The [SpatialUserObjectVectorPostprocessor](https://mooseframework.inl.gov/source/vectorpostprocessors/SpatialUserObjectVectorPostprocessor.html)
outputs the bin values for the user object to the CSV format
specified in the `[Outputs]` block. These values are ordered in the same manner
that the bins are defined.

We have stressed that the NekRS mesh does *not* respect the subchannel discretization,
or the desired number of axial averaging layers. Therefore, if we want to properly
visualize the results of the user object, we transfer the user object to two
different sub-applications
which serve the sole purpose of user object visualization. We use two different
sub-applications because one sub-app will use a mesh that perfectly represents
the *volumes* of the channels, while the second sub-app will use a mesh that perfectly
represents the *gaps* between the channels.

The application that will be used
to view the volume results on the channels is shown below.
This sub-application input
file constructs a subchannel mesh with the [HexagonalSubchannelMesh](/mesh/HexagonalSubchannelMesh.md)
to receive the user object into a variable named `average_T`. We set `solve = false` so that
no physics solve occurs in this sub-application.

!listing /tutorials/subchannel/subchannel.i

The application that will be used to view the gap results is shown below.
This sub-application input
file constructs a subchannel mesh with the [HexagonalSubchannelGapMesh](/mesh/HexagonalSubchannelGapMesh.md)
to receive the user object into a variable named `average_T`. We set `solve = false` so that
no physics solve occurs in this sub-application.

!listing /tutorials/subchannel/subchannel_gap.i

This input can be run with

```
$ mpiexec -np 8 cardinal-opt -i nek.i
```

which will run with 8 [!ac](MPI) ranks. Because we don't specify any output
block in the Nek-wrapping input file itself, the only output files are:

- `nek_out_subchannel0.e`, which shows the channel-averaged temperature on a subchannel
  discretization
- `nek_out_subchannel_gap0.e`, which shows the gap-averaged temperature on the
  gaps in a subchannel discretization

The temperature initial condition that we set
from NekRS is shown in [sub], along with the result of the subchannel volume and gap averaging. Note that
because we transferred the user object to a sub-application with a mesh that respects
our axial bins, we perfectly represent the channel-wise average temperatures.

!media subchannel_avgs2.png
  id=sub
  caption=NekRS temperature (left), subchannel-averaged temperature (middle), and gap-averaged temperature (right) computed using Cardinal user objects

For other geometries, other binning strategies can be used.
Available user objects for specifying spatial bins are:

- [HexagonalSubchannelGapBin](/userobjects/HexagonalSubchannelGapBin.md)
- [HexagonalSubchannelBin](/userobjects/HexagonalSubchannelBin.md)
- [LayeredBin](/userobjects/LayeredBin.md)
- [RadialBin](/userobjects/RadialBin.md)
