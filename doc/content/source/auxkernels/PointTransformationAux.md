# PointTransformationAux

!syntax description /AuxKernels/PointTransformationAux

## Description

Displays the $x$, $y$, or $z$ coordinates that get mapped to OpenMC. This
auxiliary kernel can be used to ensure that the
`normal`, `rotation_axis`, and `rotation_angle` parameters on
[SymmetryPointGenerator](/userobjects/SymmetryPointGenerator.md)
apply the desired transformations. In other words, this class displays the
*transformed* spatial coordinates.

## Example Input Syntax

As an example, we can visualize the transformed $x$, $y$, and $z$ coordinates
for a mapping that obeys 1/6th symmetry with a symmetry plane normal
of $\hat{n}=\hat{y}$ and a symmetry axis of $\hat{m}=\hat{z}$ with the following.

!listing test/tests/symmetry/rotation.i
  start=AuxKernels
  end=Executioner

Then, the `x` variable will show the transformed $x$ coordinate
- as shown in [xfig], the $x$ coordinate obeys 1/6th symmetry.

!media sixth_symmetry.png
  id=xfig
  caption=Example visualization of the transformed $x$, as viewed looking down the negative $z$ axis
  style=width:60%;margin-left:auto;margin-right:auto;halign:center

The OpenMC model would then need to correspond to the blue 1/6th slice in
[slice] in order for the applied `normal`, `rotation_axis`,
and `rotation_angle` to properly map to the OpenMC domain.

!media slice.png
  id=slice
  caption=Intended mapping of mesh elements to 1/6th symmetry OpenMC model, the blue slice
  style=width:30%;margin-left:auto;margin-right:auto;halign:center

!syntax parameters /AuxKernels/PointTransformationAux

!syntax inputs /AuxKernels/PointTransformationAux

!syntax children /AuxKernels/PointTransformationAux
