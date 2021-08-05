# NekSideIntegral

!syntax description /Postprocessors/NekSideIntegral

## Description

This postprocessor computes the integral of
a specified field over a boundary in the NekRS mesh,

\begin{equation}
p=\int_{\Gamma}f\ d\Gamma
\end{equation}

where $p$ is the value of the postprocessor,
$\Gamma$ is the boundary of the NekRS mesh, and
$f$ is the specified field.

!include /boundary_specs.md

!include /field_specs.md

Setting `field = unity` is equivalent to computing the area.

!include /nondimensional.md

## Example Input Syntax

As an example, the following code snippet will evaluate the area,
and side-integrated temperature, pressure, and velocity magnitude
on the boundaries of the NekRS mesh.

!listing test/tests/postprocessors/nek_side_integral/nek.i
  block=Postprocessors

!syntax parameters /Postprocessors/NekSideIntegral

!syntax inputs /Postprocessors/NekSideIntegral

!syntax children /Postprocessors/NekSideIntegral
