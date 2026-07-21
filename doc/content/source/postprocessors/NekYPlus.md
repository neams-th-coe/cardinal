# NekYPlus

!syntax description /Postprocessors/NekYPlus

## Description

This postprocessor computes the maximum, minimum, or average value of $y^+$
on given sidesets in the domain. The value of $y^+$ at a point on the
boundary is

\begin{equation}
y^+=\frac{\delta u_\tau}{\nu}
\end{equation}

where $\delta$ is the distance from the wall of the nearest GLL point,
$\nu$ is the kinematic viscosity$, and $u_\tau$ is the friction velocity,

\begin{equation}
u_\tau\equiv\sqrt{\frac{\tau_w}{\rho}}
\end{equation}

The numerator in this expression represents the viscous force on the wall,
but only the components parallel to the wall. To be clear, the wall exerts
a viscous force on the fluid expressed by a vector $\vec{f}$ with components

\begin{equation}
f_i=\tau_{ij}n_j
\end{equation}

where $n_j$ are the components of the unit outward normal $\hat{n}$ on the
wall. This vector in general has components in all directions, including some
directions perpendicular to the wall. $\tau_w$ is the magnitude of the
vector $\vec{f}$, but only the components parallel to the wall.

\begin{equation}
\tau_w\equiv\|\vec{f}-\left(\vec{f}\cdot\hat{n}\right)\hat{n}\|
\end{equation}

Or, written in tensor notation, the $i$-th component of the expression above is

\begin{equation}
f_i-\left(\vec{f}\cdot\hat{n}\right)n_i\right\tau_{ij}n_j-(\tau_{kl}n_ln_k)n_i
\end{equation}

## Example Input Syntax

As an example, the postprocessors below compute the $y^+$ on a wall
sideset in the NekRS mesh.

!listing test/tests/postprocessors/nek_yplus/nek.i
  block=Postprocessors

!syntax parameters /Postprocessors/NekYPlus

!syntax inputs /Postprocessors/NekYPlus
