# NekBinnedSideIntegral

!syntax description /UserObjects/NekBinnedSideIntegral

## Description

This user objects performs a side integral of a specified field
over a sideset in the NekRS mesh that overlap with volume "bins."
For a bin on $\Gamma_i$ (a patch of a sideset),
the user object value in that bin is

\begin{equation}
p_i=\int_{\Gamma_i}f\ d\Gamma
\end{equation}

where $p$ is the value of the user object in bin $i$ and
$f$ is the specified field. In other words, this object integrates
on a sideset in the NekRS domain, by splitting up the integral according
to a specified volume binning strategy.
To be clear, this user object is *not* evaluated on the
[NekRSMesh](/mesh/NekRSMesh.md) mesh mirror, but instead on the mesh actually
used for computation in NekRS.

!include /postprocessors/field_specs.md

Setting `field = unity` is equivalent to computing the area.

!include velocity_component.md

!include /postprocessors/nondimensional.md

!include spatial_bins.md

## Example Input Syntax

As an example, the input below defines three individual bin distributions
(named `x`, `y`, and `z` to represent 1-D binning in each
coordinate direction). These bins are then combined to integrate
over sideset 2 in $2*2*3$ equal-size spatial bins.

!listing test/tests/userobjects/side/nondimensional/nek.i
  block=UserObjects

The result of the user object can then be visualized with a
[SpatialUserObjectAux](https://mooseframework.inl.gov/source/auxkernels/SpatialUserObjectAux.html).

!syntax parameters /UserObjects/NekBinnedSideIntegral

!syntax inputs /UserObjects/NekBinnedSideIntegral

!syntax children /UserObjects/NekBinnedSideIntegral
