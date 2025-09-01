# NekPressureSurfaceForce

!syntax description /Postprocessors/NekPressureSurfaceForce

## Description

This postprocessor computes the integral of the pressure component of the
stress tensor over a boundary, multiplied by negative 1 in order to compute
the force that the fluid exerts ON the boundary,

\begin{equation}
\begin{aligned}
r_i=&\ -\int_{\Gamma}-P\delta_{ij}n_j\ d\Gamma\\
=&\ -\int_{\Gamma}-Pn_i\ d\Gamma\\
=&\ \int_{\Gamma}Pn_i\ d\Gamma\\
\end{aligned}
\end{equation}

where $\Gamma$ is the boundary of the NekRS mesh and
$\hat{n}$ is the unit normal of the boundary with components $n_i$.
This postprocessor will either return the $x$ component, $y$ component,
$z$ component, or magnitude of this force.

!include /boundary_specs.md

!include /nondimensional.md

!alert tip
This postprocessor computes the stress integrated over a surface (i.e. a force).
If you instead want the pointwise pressure on the surface, you can add a
[NekFieldVariable](NekFieldVariable.md) in order to write NekRS's pressure
into an [AuxVariable](AuxVariables/index.md). You can then use this pointwise pressure
as a pointwise force boundary condition in each momentum conservation equation
using three [CoupledPressureBC.md](CoupledPressureBC.md) objects.

## Example Input Syntax

As an example, the postprocessors below compute the pressure surface force
on various sidesets in the NekRS mesh.

!listing test/tests/postprocessors/nek_pressure_surface_force/nek.i
  block=Postprocessors

!syntax parameters /Postprocessors/NekPressureSurfaceForce

!syntax inputs /Postprocessors/NekPressureSurfaceForce
