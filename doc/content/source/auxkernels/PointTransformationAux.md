# PointTransformationAux

!syntax description /AuxKernels/PointTransformationAux

## Description

Displays the $x$, $y$, or $z$ coordinates that get mapped to OpenMC. This
auxiliary kernel can be used to ensure that the
`symmetry_plane_normal`, `symmetry_axis`, and `symmetry_angle` parameters on
[OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md)
apply the desired transformations. In other words, this class only displays the
*transformed* spatial coordinates that get sent into the
`openmc::exhaustive_find_cell` routine.

## Example Input Syntax

As an example, we can visualize the transformed $x$, $y$, and $z$ coordinates
for an OpenMC mapping that obeys 1/6th symmetry with a symmetry plane normal
of $\hat{n}=\hat{y}$ and a symmetry axis of $\hat{m}=\hat{z}$ with the following.

!listing test/tests/symmetry/rotation.i
  start=AuxKernels
  end=Executioner

Then, the `x` variable will show the transformed $x$ coordinate that gets mapped
to OpenMC - as shown in [xfig], the $x$ coordinate obeys 1/6th symmetry.

!media sixth_symmetry.png
  id=xfig
  caption=Example visualization of the transformed $x$ that gets mapped to this particular OpenMC model, as viewed looking down the negative $z$ axis
  style=width:60%;margin-left:auto;margin-right:auto;halign:center

The OpenMC model would then need to correspond to the blue 1/6th slice in
[slice] in order for the applied `symmetry_plane_normal`, symmetry_axis`,
and `symmetry_angle` to properly map to the OpenMC domain.

!media slice.png
  id=slice
  caption=Intended mapping of mesh elements to 1/6th symmetry OpenMC model, the blue slice
  style=width:30%;margin-left:auto;margin-right:auto;halign:center

!syntax parameters /AuxKernels/PointTransformationAux

!syntax inputs /AuxKernels/PointTransformationAux

!syntax children /AuxKernels/PointTransformationAux
