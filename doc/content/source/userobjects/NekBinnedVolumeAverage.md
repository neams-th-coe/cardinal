# NekBinnedVolumeAverage

!syntax description /UserObjects/NekBinnedVolumeAverage

## Description

This user objects performs a volume average of a specified field
over bins defined on the NekRS mesh. For a bin on $\Omega_i$,
the user object value in that bin is

\begin{equation}
p_i=\frac{\int_{\Omega_i}f\ d\Omega}{\int_{\Omega_i}d\Omega}
\end{equation}

where $p$ is the value of the user object in bin $i$ and
$f$ is the specified field.
To be clear, this user object is *not* evaluated on the
[NekRSMesh](/mesh/NekRSMesh.md) mesh mirror, but instead on the mesh actually
used for computation in NekRS.

!include /postprocessors/field_specs.md

Setting `field = unity` is equivalent to computing
1, since the numerator will be exactly equal to the denominator (`unity` is
of more use for other postprocessors).

!include /postprocessors/nondimensional.md

!include spatial_bins.md

## Example Input Syntax

As an example, the input below defines three individual bin distributions
(named `x_bins`, `y_bins`, and `z_bins` to represent 1-D binning in each
coordinate direction). These bins are then combined to average
over the NekRS mesh in $3*3*12$ equal-size spatial bins.

!listing test/tests/userobjects/layered_layered/nek.i
  block=UserObjects

The result of the user object can then be visualized with a
[SpatialUserObjectAux](https://mooseframework.inl.gov/source/auxkernels/SpatialUserObjectAux.html).

!syntax parameters /UserObjects/NekBinnedVolumeAverage

!syntax inputs /UserObjects/NekBinnedVolumeAverage

!syntax children /UserObjects/NekBinnedVolumeAverage
