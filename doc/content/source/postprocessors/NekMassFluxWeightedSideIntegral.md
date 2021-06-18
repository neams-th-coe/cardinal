# NekMassFluxWeightedSideIntegral

!syntax description /postprocessors/NekMassFluxWeightedSideIntegral

## Description

This postprocessor computes the mass-flux-weighted integral of
a specified field over a boundary in the nekRS mesh,

\begin{equation}
p=\int_{\Gamma}f\rho\vec{V}\cdot\hat{n}\ d\Gamma
\end{equation}

where $p$ is the value of the postprocessor,
$\Gamma$ is the boundary of the nekRS mesh (*not* the mesh mirror constructed with
NekRSMesh), $\rho$ is the fluid density, $\vec{V}$ is the fluid velocity,
$f$ is the specified field,
and $\hat{n}$ is the surface unit normal. The boundaries over which to integrate in
the nekRS mesh are specified with the `boundary` parameter; these boundaries
are the sidesets in nekRS's mesh (i.e. the `.re2` file). The field is specified with the `field` parameter, which may be one of
`pressure`, `temperature`, or `unity`. Setting `field = unity` is equivalent to computing
the mass flowrate.

## Example Input Syntax

As an example, the following code snippet will evaluate the
mass-flux-weighted temperature (for `field = temperature`)
on the boundaries of the nekRS mesh.

!listing test/tests/postprocessors/nek_weighted_side_integral/nek.i
  block=Postprocessors

!syntax parameters /postprocessors/NekMassFluxWeightedSideIntegral

!syntax inputs /postprocessors/NekMassFluxWeightedSideIntegral

!syntax children /postprocessors/NekMassFluxWeightedSideIntegral
