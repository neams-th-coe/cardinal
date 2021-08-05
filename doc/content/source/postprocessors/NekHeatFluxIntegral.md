# NekHeatFluxIntegral

!syntax description /Postprocessors/NekHeatFluxIntegral

## Description

This postprocessor computes the heat flux over a boundary in the NekRS mesh,

\begin{equation}
p=\int_{\Gamma}-k\nabla T\cdot\hat{n}\ d\Gamma
\end{equation}

where $p$ is the value of the postprocessor,
$\Gamma$ is the boundary of the NekRS mesh,
$k$ is the fluid thermal conductivity, $T$ is the fluid temperature,
and $\hat{n}$ is the surface unit normal.

!include /boundary_specs.md

If running NekRS in non-dimensional form (and you have indicated the
appropriate nondimensional scales by setting `nondimensional = true`
for [NekRSProblem](/problems/NekRSProblem.md)), then the value of this postprocessor
is shown in *dimensional* units.

## Example Input Syntax

As an example, the two heat flux integrals below will evaluate the heat flux
over two different boundaries in NekRS's mesh.

!listing tutorials/sfr_7pin/nek.i
  block=Postprocessors

!syntax parameters /Postprocessors/NekHeatFluxIntegral

!syntax inputs /Postprocessors/NekHeatFluxIntegral

!syntax children /Postprocessors/NekHeatFluxIntegral
