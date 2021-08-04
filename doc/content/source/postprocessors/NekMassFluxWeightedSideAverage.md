# NekMassFluxWeightedSideAverage

!syntax description /postprocessors/NekMassFluxWeightedSideAverage

## Description

This postprocessor computes the mass-flux-weighted average of
a specified field over a boundary in the NekRS mesh,

\begin{equation}
p=\frac{\int_{\Gamma}f\rho\vec{V}\cdot\hat{n}\ d\Gamma}{\int_{\Gamma}\rho\vec{V}\cdot\hat{n}\ d\Gamma}
\end{equation}

where $p$ is the value of the postprocessor,
$\Gamma$ is the boundary of the NekRS mesh,
$\rho$ is the fluid density, $\vec{V}$ is the fluid velocity,
$f$ is the specified field,
and $\hat{n}$ is the surface unit normal.

!include /boundary_specs.md

!include /field_specs.md

Setting `field = unity` is equivalent to computing
1, since the numerator will be exactly equal to the denominator (`unity` is
of more use for other postprocessors).

If running NekRS in non-dimensional form (and you have indicated the
appropriate nondimensional scales by setting `nondimensional = true`
for [NekRSProblem](/problems/NekRSProblem.md)), then the value of this postprocessor
is shown in *dimensional* units.

## Example Input Syntax

As an example, the following code snippet will evaluate the mass-flux-weighted
average of temperature over the outlet boundary of NekRS's mesh.

!listing tutorials/sfr_multiphysics/nek.i
  block=Postprocessors

!syntax parameters /postprocessors/NekMassFluxWeightedSideAverage

!syntax inputs /postprocessors/NekMassFluxWeightedSideAverage

!syntax children /postprocessors/NekMassFluxWeightedSideAverage
