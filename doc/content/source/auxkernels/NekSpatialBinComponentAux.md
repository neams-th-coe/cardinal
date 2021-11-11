# NekSpatialBinComponentAux

!syntax description /AuxKernels/NekSpatialBinComponentAux

## Description

Displays a component ($x$, $y$, or $z$) of a binned user object
that integrates/averages a vector NekRS solution field. This auxiliary kernel
is primarily for visualization purposes so that a "glyph" filter
can be used in Paraview, or equivalent visualization software, for showing
a vector-type postprocessed quantity.

## Example Input Syntax

As an example, suppose we use a [NekBinnedSideAverage](/userobjects/NekBinnedSideAverage.md)
user object to compute the average velocity normal to a set of planes within the
NekRS domain. The actual result of the [NekBinnedSideAverage](/userobjects/NekBinnedSideAverage.md)
user object will be the *magnitude* of the average normal velocity in the direction
of the unit normal vectors. But to be able to visualize these vectors
in Paraview, we can use the `NekSpatialBinComponentAux` to extract each of the
three components into variables, that we can then show in a glyph mode.

!listing test/tests/userobjects/hexagonal_gap_layered/normals/nek.i
  start=AuxVariables
  end=MultiApps

In the example above, you can use a "glyph" filter in Paraview to visualize
the directional component of [NekBinnedSideAverage](/userobjects/NekBinnedSideAverage.md),
as shown below. In this example, the velocity is a swirl velocity in the counterclockwise
direction with an angular
component that increases with $r$ and zero radial component.

!media nek_component_aux.png
  id=bundle
  caption=NekRS velocity components along the gap-normal directions, visualized with a glyph filter in Paraview by first extracting the components of the `avg_velocity_component` user object with a `NekSpatialBinComponentAux`
  style=width:60%;margin-left:auto;margin-right:auto;halign:center

!syntax parameters /AuxKernels/NekSpatialBinComponentAux

!syntax inputs /AuxKernels/NekSpatialBinComponentAux

!syntax children /AuxKernels/NekSpatialBinComponentAux
