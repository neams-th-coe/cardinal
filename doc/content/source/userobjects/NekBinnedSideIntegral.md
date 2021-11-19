# NekBinnedSideIntegral

!syntax description /UserObjects/NekBinnedSideIntegral

## Description

This user objects performs a side integral of a specified field
over face "bins" defined on the NekRS mesh. For illustration, consider
a rectangular face aligned with the $x$ axis on $x_0\leq x\leq x_1$,
$y=y_0$, and $z_0\leq z\leq z_1$. Because this face may not necessarily
align with the faces of NekRS's mesh elements, a 2-D face integral is approximated
as a volume integral over a prism with a face-normal thickness of
`gap_thickness`, denoted here as $\Delta$:

\begin{equation}
[eq:1]
p=\frac{\int_{x_0}^{x_1}\int_{y_0-\frac{\Delta}{2}}^{y_0+\frac{\Delta}{2}}\int_{z_0}^{z_1}f\ dxdydz}{\int_{y_0-\frac{\Delta}{2}}^{y_0+\frac{\Delta}{2}}dy}
\end{equation}

where $p$ is the value of the user object and
$f$ is the specified field.
To be clear, this user object is *not* evaluated on the
[NekRSMesh](/mesh/NekRSMesh.md) mesh mirror, but instead on the mesh actually
used for computation in NekRS.

!alert! warning
This user object can produce very inaccurate results due to the nature of the
averaging performed in [eq:1]. The averaging in [eq:1] is essentially the same as applying
a delta function that is unity at nodes within a small $\frac{\Delta}{2}$ distance
of the face, and zero for all other nodes. *However*, this implementation is technically
no different from a different selection of $\Delta$ which *does not change the number of GLL points "hit"*.
For illustration, consider the setup shown in [fig1]. While [eq:1] shows that the normalization
to an *area* is obtained by dividing by an integral, in implementation we do not have a means
to directly compute this integral in the denominator. Instead, we divide the numerator in
[eq:1] by the `gap_thickness`.

!media side_integral.png
  id=fig1
  caption=Illustration of potential inaccuracies with this object; black lines show the connections between GLL points and the shaded red areas show the contributing volumes to a face
  style=width:40%;margin-left:auto;margin-right:auto

However, if the effective integrating volume does not
"hit" GLL points that are $\frac{\Delta}{2}$ away from the gap, then the effective thickness
of the integral is technically indistinguishable from other values of $\Delta$ that
hit the same number of GLL points. Because the mesh may be unstructured and not be oriented
in any convenient way with respect to the faces (unlike the structured depiction in [fig1]),
there is no other value of `gap_thickness` by which we could divide [eq:1] to correctly
obtain areas.

Therefore, we recommend only using this class if you know that your mesh is structured
and you have selected a `gap_thickness` such that for every bin, your GLL points would
exactly lie on the surface of a rectangular prism of width $\Delta$. Instead, you should
use the [NekBinnedSideAverage](/userobjects/NekBinnedSideAverage.md) object to get an
average over the face, and then multiply that value by the known area of the gap
(i.e. don't use this object to try to calculate the gap area).
!alert-end!

!include /postprocessors/field_specs.md

Setting `field = unity` is equivalent to computing the surface area.

!include velocity_component.md

!include /postprocessors/nondimensional.md

The bins (which can be a combination of side and volume bins) are specified
by providing a list of binning user objects with the `bin` parameter. This user object
requires providing one and only one "side" bin, which defines the planes over which to integrate.
The available user objects for specifying *side* spatial bins are:

- [HexagonalSubchannelGapBin](/userobjects/HexagonalSubchannelGapBin.md)

The side bins can then be combined with any number of "volume" bins to further subdivide
the domain. Available user objects for specifying the volume spatial bins are:

- [HexagonalSubchannelBin](/userobjects/HexagonalSubchannelBin.md)
- [LayeredBin](/userobjects/LayeredBin.md)
- [RadialBin](/userobjects/RadialBin.md)

If more than one bin is provided, then the bins are taken as the
product of each individual bin distribution.

It is recommended to set `map_space_by_qp = true` for this user object; otherwise,
all the GLL points in an element with centroid closer than $\Delta/2$ to the gap
plane will contribute to the plane integral.

## Example Input Syntax

As an example, the input below defines side bins as the gap planes in
a hexagonal lattice of pins (named `subchannel_binning`). These side bins
are then combined with a [LayeredBin](/userobjects/LayeredBin.md) to perform
integrals over the gap planes in `num_layers` axial segments.

!listing test/tests/userobjects/hexagonal_gap_layered/nek.i
  block=UserObjects

The result of the user object can then be visualized with a
[SpatialUserObjectAux](https://mooseframework.inl.gov/source/auxkernels/SpatialUserObjectAux.html).

!syntax parameters /UserObjects/NekBinnedSideIntegral

!syntax inputs /UserObjects/NekBinnedSideIntegral

!syntax children /UserObjects/NekBinnedSideIntegral
