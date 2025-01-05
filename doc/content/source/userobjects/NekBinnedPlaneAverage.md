# NekBinnedPlaneAverage

!syntax description /UserObjects/NekBinnedPlaneAverage

## Description

This user objects performs a side average of a specified field
over face "bins" defined on the NekRS mesh. For illustration, consider
a rectangular face aligned with the $x$ axis on $x_0\leq x\leq x_1$,
$y=y_0$, and $z_0\leq z\leq z_1$. Because this face may not necessarily
align with the faces of NekRS's mesh elements, a 2-D face average is approximated
as a volume average over a prism with a face-normal thickness of
`gap_thickness`, denoted here as $\Delta$:

\begin{equation}
p=\frac{\int_{x_0}^{x_1}\int_{y_0-\frac{\Delta}{2}}^{y_0+\frac{\Delta}{2}}\int_{z_0}^{z_1}f\ dxdydz}{\int_{x_0}^{x_1}\int_{y_0-\frac{\Delta}{2}}^{y_0+\frac{\Delta}{2}}\int_{z_0}^{z_1}dxdydz}
\end{equation}

where $p$ is the value of the user object and
$f$ is the specified field.
To be clear, this user object is *not* evaluated on the
[NekRSMesh](/mesh/NekRSMesh.md) mesh mirror, but instead on the mesh actually
used for computation in NekRS.

!include /postprocessors/field_specs.md

Setting `field = unity` is equivalent to computing
1, since the numerator will be exactly equal to the denominator (`unity` is
of more use for other user objects).

!include velocity_component.md

!include /postprocessors/nondimensional.md

The bins (which can be a combination of side and volume bins) are specified
by providing a list of binning user objects with the `bin` parameter. This user object
requires providing one and only one "side" bin, which defines the planes over which to integrate.
The available user objects for specifying *side* spatial bins are:

- [HexagonalSubchannelGapBin](/userobjects/HexagonalSubchannelGapBin.md)
- [LayeredGapBin](/userobjects/LayeredGapBin.md)

The side bins can then be combined with any number of "volume" bins to further subdivide
the domain. Available user objects for specifying the volume spatial bins are:

- [HexagonalSubchannelBin](/userobjects/HexagonalSubchannelBin.md)
- [LayeredBin](/userobjects/LayeredBin.md)
- [RadialBin](/userobjects/RadialBin.md)

If more than one bin is provided, then the bins are taken as the
product of each individual bin distribution.

It is recommended to set `map_space_by_qp = true` for this user object; otherwise,
all the GLL points in an element with centroid closer than $\Delta/2$ to the gap
plane will contribute to the plane average.

## Example Input Syntax

As an example, the input below defines side bins as the gap planes in
a hexagonal lattice of pins (named `subchannel_binning`). These side bins
are then combined with a [LayeredBin](/userobjects/LayeredBin.md) to perform
averages over the gap planes in `num_layers` axial segments.

!listing test/tests/userobjects/hexagonal_gap_layered/nek.i
  block=UserObjects

The result of the user object can then be visualized with a
[SpatialUserObjectAux](https://mooseframework.inl.gov/source/auxkernels/SpatialUserObjectAux.html).

!syntax parameters /UserObjects/NekBinnedPlaneAverage

!syntax inputs /UserObjects/NekBinnedPlaneAverage
