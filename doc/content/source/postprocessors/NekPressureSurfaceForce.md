# NekPressureSurfaceForce

!syntax description /Postprocessors/NekPressureSurfaceForce

## Description

This postprocessor computes the integral of the pressure component of the
stress tensor over a boundary,

\begin{equation}
r=\int_{\Gamma}-P\hat{n}\cdot\hat{d}\ d\Gamma
\end{equation}

where $r$ is the value of the postprocessor,
$\Gamma$ is the boundary of the NekRS mesh,
$\hat{n}$ is the unit normal of the boundary, and
$\hat{d}$ is a user-specified direction vector. For example, if you set
`direction = '1 0 0'`, then you will compute the $x$-component of the
pressure drag on the given sideset.

!include /boundary_specs.md

If running NekRS in non-dimensional form (and you have indicated the
appropriate nondimensional scales by setting `nondimensional = true`,
for the `[Problem]`, then the value of this postprocessor is shown
in *dimensional* units.

## Example Input Syntax

As an example, the postprocessors below compute the pressure surface force
on various sidesets in the NekRS mesh.

!listing test/tests/postprocessors/nek_pressure_surface_force/nek.i
  block=Postprocessors

!syntax parameters /Postprocessors/NekPressureSurfaceForce

!syntax inputs /Postprocessors/NekPressureSurfaceForce

!syntax children /Postprocessors/NekPressureSurfaceForce
