# NekMassFluxWeightedSideIntegral

!syntax description /Postprocessors/NekMassFluxWeightedSideIntegral

## Description

This postprocessor computes the mass-flux-weighted integral of
a specified field over a boundary in the NekRS mesh,

\begin{equation}
p=\int_{\Gamma}f\rho\vec{V}\cdot\hat{n}\ d\Gamma
\end{equation}

where $p$ is the value of the postprocessor,
$\Gamma$ is the boundary of the NekRS mesh,
$\rho$ is the fluid density, $\vec{V}$ is the fluid velocity,
$f$ is the specified field,
and $\hat{n}$ is the surface unit normal.

!include /boundary_specs.md

!include /field_specs.md

Setting `field = unity` is equivalent to computing the mass flowrate.

!include /nondimensional.md

## Example Input Syntax

As an example, the following code snippet will evaluate the
mass flowrate on the inlet and outlet boundaries of the NekRS mesh.

!listing tutorials/fhr_reflector/cht/nek.i
  block=Postprocessors

!syntax parameters /Postprocessors/NekMassFluxWeightedSideIntegral

!syntax inputs /Postprocessors/NekMassFluxWeightedSideIntegral

!syntax children /Postprocessors/NekMassFluxWeightedSideIntegral
