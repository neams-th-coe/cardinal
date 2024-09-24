# NekPressureSurfaceForce

!syntax description /Postprocessors/NekPressureSurfaceForce

## Description

This postprocessor computes the integral of the pressure component of the
stress tensor over a boundary, multiplied by negative 1 in order to compute
the force that the fluid exerts ON the boundary,

\begin{equation}
r_i=&\ -\int_{\Gamma}-Pn_i\ d\Gamma\\
=&\ \int_{\Gamma}Pn_i\ d\Gamma\\
\end{equation}

where $\Gamma$ is the boundary of the NekRS mesh and
$\hat{n}$ is the unit normal of the boundary with components $n_i$.
This postprocessor will either return the $x$ component, $y$ component,
$z$ component, or magnitude of this force.

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
