# NekDiffusiveFluxIntegral

!syntax description /Postprocessors/NekDiffusiveFluxIntegral

## Description

This postprocessor computes a diffusive flux over a boundary in the NekRS mesh,

\begin{equation}
p=\int_{\Gamma}-k\nabla T\cdot\hat{n}\ d\Gamma
\end{equation}

where $p$ is the value of the postprocessor,
$\Gamma$ is the boundary of the NekRS mesh,
$k$ is the diffusion coefficient, $T$ is the scalar,
and $\hat{n}$ is the surface unit normal.

!include /boundary_specs.md

!include /nondimensional.md

## Example Input Syntax

As an example, the `pin_flux_in_nek` and `duct_flux_in_nek` postprocessors
below will evaluate the heat flux
over two different boundaries in NekRS's mesh.

!listing tutorials/sfr_7pin/nek.i
  block=Postprocessors

!syntax parameters /Postprocessors/NekDiffusiveFluxIntegral

!syntax inputs /Postprocessors/NekDiffusiveFluxIntegral
