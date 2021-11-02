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

## Binned Spatial Postprocessors

Cardinal contains features for postprocessing the NekRS solution in spatial
"bins" using user objects. Available user objects include:

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
subchannel-averaged temperatures on a mesh that reflects a subchannel discretization.
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

Of note are the user objects, multiapps, and transfers. We will use a user object to
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

The [SpatialUserObjectVectorPostprocessor](https://mooseframework.inl.gov/source/vectorpostprocessors/SpatialUserObjectVectorPostprocessor.html)
outputs the bin values for the user object to the CSV format
specified in the `[Outputs]` block. These values are ordered in the same manner
that the bins are defined.

We have stressed that the NekRS mesh does *not* respect the subchannel discretization,
or the desired number of axial averaging layers. Therefore, if we want to properly
visualize the results of the user object, we transfer the user object to a sub-application
that serves the sole purpose of user object visualization.

!listing /tutorials/subchannel/subchannel.i

This sub-application input
file constructs a subchannel mesh with the [HexagonalSubchannelMesh](/mesh/HexagonalSubchannelMesh.md)
to receive the user object into a variable named `average_T`. We set `solve = false` so that
no physics solve occurs in this sub-application.
This input can be run with

```
$ mpiexec -np 8 cardinal-opt -i nek.i
```

which will run with 8 [!ac](MPI) ranks. Because we don't specify any output
block in the Nek-wrapping input file itself, the only output file is the `nek_out_subchannel0.e`
file (along with whatever field files NekRS writes itself).

The temperature initial condition that we set
from NekRS is shown in [sub], along with the result of the subchannel averaging. Note that
because we transferred the user object to a sub-application with a mesh that respects
our axial bins, we perfectly represent the channel-wise average temperatures.

!media subchannel_avgs.png
  id=sub
  caption=NekRS temperature (left) and subchannel-averaged temperature (right) computed using Cardinal user objects

For other geometries, other binning strategies can be used.
Available user objects for specifying spatial bins are:

- [HexagonalSubchannelBin](/userobjects/HexagonalSubchannelBin.md)
- [LayeredBin](/userobjects/LayeredBin.md)
- [RadialBin](/userobjects/RadialBin.md)
