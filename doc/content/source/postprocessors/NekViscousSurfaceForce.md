# NekViscousSurfaceForce

!syntax description /Postprocessors/NekViscousSurfaceForce

## Description

This postprocessor computes the integral of the frictional component of the
stress tensor over a boundary, multiplied by negative 1 in order to compute
the force that the fluid exerts ON the boundary,

\begin{equation}
r_i=-\int_{\Gamma}2\mu e_{ij}n_j\ d\Gamma
\end{equation}

where $\Gamma$ is the boundary of the NekRS mesh and
$\hat{n}$ is the unit normal of the boundary with components $n_j$.
This postprocessor will either return the $x$ component, $y$ component,
$z$ component, or magnitude of this force.

!include /boundary_specs.md

## Example Input Syntax

As an example, the postprocessors below compute the viscous surface force
on various sidesets in the NekRS mesh.

!listing test/tests/postprocessors/nek_viscous_surface_force/nek.i
  block=Postprocessors

!syntax parameters /Postprocessors/NekViscousSurfaceForce

!syntax inputs /Postprocessors/NekViscousSurfaceForce
