# NekMassFluxWeightedSideAverage

!syntax description /Postprocessors/NekMassFluxWeightedSideAverage

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

!include /nondimensional.md

## Example Input Syntax

As an example, the `outlet_T` postprocessor evaluates the mass-flux-weighted
average of temperature over the outlet boundary of NekRS's mesh.

!listing tutorials/pincell_multiphysics/nek.i
  block=Postprocessors

!syntax parameters /Postprocessors/NekMassFluxWeightedSideAverage

!syntax inputs /Postprocessors/NekMassFluxWeightedSideAverage

!syntax children /Postprocessors/NekMassFluxWeightedSideAverage
