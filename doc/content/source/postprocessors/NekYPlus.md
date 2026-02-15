# NekYPlus

!syntax description /Postprocessors/NekYPlus

## Description

This postprocessor computes the maximum, minimum, or average value of $y^+$
on given sidesets in the domain. The value of $y^+$ at a point on the
boundary is

\begin{equation}
y^+=\frac{\delta u_*}{\nu}
\end{equation}

where $\delta$ is the distance from the wall of the nearest GLL point,
$\nu$ is the kinematic viscosity$, and $u_*$ is the friction velocity,

\begin{equation}
u_*\equiv\sqrt{\frac{\tau_w}{\rho}}
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

## Computing the Distance to the Wall

To use this postprocessor, you need to compute the wall distance in NekRS, which
can be done in the `.usr` file. First, in `usrdat2` you need to explicitly set
the `cbc` array to `'W  '` for any sidesets which correspond to walls. The `1`
in `cbc(ifc,iel,1)` corresponds to the sideset ID.

Then, you need to compute the wall distance in `usrdat3` by calling the `distf`
function. The `1` in `distf(ywd,1,'W  ',w1,w2,w3,w4,w5)` is the sideset ID corresponding
to the wall. If you have multiple walls in the problem, simply call `dist` multiple times.
Finally, you need to expose the wall distance to the C++ side of NekRS by assigning
the `nrs_scptr`. The `1` in `nrs_scptr(1)` is the `wall_distance_index` parameter
that this object will ask you to provide.

!listing test/tests/postprocessors/nek_yplus/pipe.usr language=fortran

## Example Input Syntax

As an example, the postprocessors below compute the $y^+$ on a wall
sideset in the NekRS mesh.

!listing test/tests/postprocessors/nek_yplus/nek.i
  block=Postprocessors

!syntax parameters /Postprocessors/NekYPlus

!syntax inputs /Postprocessors/NekYPlus
