# NekHeatFluxIntegral

!syntax description /postprocessors/NekHeatFluxIntegral

## Description

This postprocessor computes the heat flux over a boundary in the nekRS mesh,

\begin{equation}
p=\int_{\Gamma}-k\nabla T\cdot\hat{n}\ d\Gamma
\end{equation}

where $p$ is the value of the postprocessor,
$\Gamma$ is the boundary of the nekRS mesh (*not* the mesh mirror constructed with
NekRSMesh), $k$ is the fluid thermal conductivity, $T$ is the fluid temperature,
and $\hat{n}$ is the surface unit normal. The boundaries over which to integrate in
are specified with the `boundary` parameter; these boundaries
are the sidesets in nekRS's mesh (i.e. the `.re2` file).

If running nekRS in non-dimensional form (and you have indicated the
appropriate nondimensional scales by setting `nondimensional = true`
for [NekRSProblem](/problems/NekRSProblem.md)), then the value of this postprocessor
is shown in *dimensional* units.

## Example Input Syntax

As an example, the following code snippet will evaluate the heat flux
over each boundary in nekRS's mesh.

!listing test/tests/postprocessors/nek_heat_flux_integral/nek.i
  block=Postprocessors

!syntax parameters /postprocessors/NekHeatFluxIntegral

!syntax inputs /postprocessors/NekHeatFluxIntegral

!syntax children /postprocessors/NekHeatFluxIntegral
