# NekSideIntegral

!syntax description /postprocessors/NekSideIntegral

## Description

This postprocessor computes the integral of
a specified field over a boundary in the nekRS mesh,

\begin{equation}
p=\int_{\Gamma}f\ d\Gamma
\end{equation}

where $p$ is the value of the postprocessor,
$\Gamma$ is the boundary of the nekRS mesh (*not* the mesh mirror constructed with
NekRSMesh), and
$f$ is the specified field.
The boundaries over which to integrate in
the nekRS mesh are specified with the `boundary` parameter; these boundaries
are the sidesets in nekRS's mesh (i.e. the `.re2` file). The field is specified with the `field` parameter, which may be one of
`pressure`, `temperature`, or `unity`. Setting `field = unity` is equivalent to computing
the area.

If running nekRS in non-dimensional form (and you have indicated the
appropriate nondimensional scales by setting `nondimensional = true`
for [NekRSProblem](/problems/NekRSProblem.md)), then the value of this postprocessor
is shown in *dimensional* units.

## Example Input Syntax

As an example, the following code snippet will evaluate the area (for `field = unity`),
side-integrated temperature (for `field = temperature`), and side-integrated pressure
(for `field = pressure`)
on the boundaries of the nekRS mesh.

!listing test/tests/postprocessors/nek_side_integral/nek.i
  block=Postprocessors

!syntax parameters /postprocessors/NekSideIntegral

!syntax inputs /postprocessors/NekSideIntegral

!syntax children /postprocessors/NekSideIntegral
