# Data Transfers in MOOSE

In this tutorial, you will learn how to:

- Select a Transfer appropriate for your simulation

To access this tutorial,

```
cd cardinal/tutorials/transfers
```

The MOOSE [Transfer system](https://mooseframework.inl.gov/syntax/Transfers/index.html)
is used to send data between applications. In Cardinal, these transfers are used to
communicate fields between NekRS, OpenMC, and MOOSE. When using Cardinal, we are most often
communicating 3-D fields (temperature, heat flux, power, etc.) between our MultiApps, so
we typically only use a subset of the [Transfers](https://mooseframework.inl.gov/syntax/Transfers/index.html)
available in MOOSE. When selecting a transfer, these are some considerations to be aware of;
this page provides example use cases for all of the applicable field-based transfers in MOOSE.

This tutorial will use two generic MOOSE applications, because the advice on this page is
agnostic of whether the physics solve is performed with OpenMC, NekRS, or another MOOSE
application - the main requirement in selecting a tranfser for a field variable is an
understanding of the source/target meshes for the transfer. We will consider the most generic
use case of a transfer between two entirely different meshes that in some regions of space
do not overlap one another. In Cardinal,
we often encounter this due to NekRS's very high-order
solution (many degrees of freedom per element allows fairly coarse elements).
For each case, we will walk through all the applicable MOOSE transfers.

We will transfer data between two cylinders, one with
a much finer mesh than the other. First, generate the meshes:

```
cardinal-opt -i mesh1.i --mesh-only
cardinal-opt -i mesh2.i --mesh-only
```

The two meshes are shown in [volume_meshes]; the wireframe overlay on the right shows that,
while both meshes represent a cylinder with the same outer radius, that due to different
azimuthal refinement, there is not perfect spatial overlap of the meshes.

!media volume_meshes.png
  id=volume_meshes
  caption=Meshes to be used for data transfers in this tutorial
  style=width:80%;margin-left:auto;margin-right:auto

For all cases,
we will send a variable from mesh 1 to mesh 2. We simply set this variable to a generic function:

!listing /tutorials/transfers/main.i
  start=Variables
  end=MultiApps

We will then send this variable using a few different transfers to a sub-application.

!listing /tutorials/transfers/main.i
  block=Transfers

## [MultiAppGeometricInterpolationTransfer [!icon!link]](https://mooseframework.inl.gov/source/transfers/MultiAppGeometricInterpolationTransfer.html)

The [MultiAppGeometricInterpolationTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppGeometricInterpolationTransfer.html) transfers the nearest node's source variable to the nearest node on the target mesh
using mesh interpolation. Any non-overlapping domains are set values using extrapolation.

!media volume_interpolation.png
  id=volume_interpolation
  caption=Results of a `MultiAppGeometricInterpolationTransfer` from the main app to the sub app
  style=width:80%;margin-left:auto;margin-right:auto

## [MultiAppShapeEvaluationTransfer [!icon!link]](https://mooseframework.inl.gov/source/transfers/MultiAppShapeEvaluationTransfer.html)

The [MultiAppShapeEvaluationTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppShapeEvaluationTransfer.html)
queries the finite element solution from the source variable at each receiving point in
the receiving application. Any non-overlapping domains will simply be set to a value of zero,
so this transfer is only recommended for meshes that share the same outer boundary.

!media volume_mesh_function.png
  id=volume_mesh_function
  caption=Results of a `MultiAppShapeEvaluationTransfer` from the main app to the sub app
  style=width:80%;margin-left:auto;margin-right:auto

## [MultiAppGeneralFieldNearestLocationTransfer [!icon!link]](https://mooseframework.inl.gov/source/transfers/MultiAppGeneralFieldNearestLocationTransfer.html)

The [MultiAppGeneralFieldNearestLocationTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppGeneralFieldNearestLocationTransfer.html) transfers a variable between the target and source domains using a nearest node lookup.
For performance, we highly recommend setting the `fixed_meshes = true` parameter to cache
the nearest node matches after the first transfer.

!media volume_nearest_node.png
  id=volume_nearest_node
  caption=Results of a `MultiAppGeneralFieldNearestLocationTransfer` from the main app to the sub app
  style=width:80%;margin-left:auto;margin-right:auto

## [MultiAppProjectionTransfer [!icon!link]](https://mooseframework.inl.gov/source/transfers/MultiAppProjectionTransfer.html)

The [MultiAppProjectionTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppProjectionTransfer.html) transfers a variable between the target and source domains using a projection mapping.
For performance, we highly recommend setting the `fixed_meshes = true` parameter to cache
the nearest node matches after the first transfer.

!media volume_projection.png
  id=volume_projection
  caption=Results of a `MultiAppProjectionTransfer` from the main app to the sub app
  style=width:80%;margin-left:auto;margin-right:auto

## [MultiAppVariableValueSampleTransfer [!icon!link]](https://mooseframework.inl.gov/source/transfers/MultiAppVariableValueSampleTransfer.html)

The [MultiAppVariableValueSampleTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppVariableValueSampleTransfer.html) transfers the value of a variable within the source application at each
receiver application's position in the `positions` parameter on the MultiApp to a field variable.
This transfer is mostly meant for multiscale simulations, where you want to send one value from a
main application to $N$ sub-applications. The value sent to each sub-application is received as a
singly-valued variable by evaluating the source variable at the application's position.

!media volume_sample.png
  id=volume_sample
  caption=Results of a `MultiAppVariableValueSampleTransfer` from the main app to the sub app
  style=width:80%;margin-left:auto;margin-right:auto
