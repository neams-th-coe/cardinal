# Low-Mach Test

The low-Mach governing equations are obtained by filtering acoustic waves from the fully compressible Navier--Stokes equations.
The pressure is decomposed into a spatially uniform, leading-order thermodynamic component and a first-order hydrodynamic component that appears in the momentum equation [!citep](tomboulides1997numerical).
The low-Mach formulation is applicable to low-speed flows with significant density variations, such as reactive flows and natural convection, where thermal expansion must be captured while acoustic waves are neglected.

## Problem description

The *lowMach* case is adopted from Tomboulides *et al.* [!citep](tomboulides1998quasi).
The problem is a nontrivial, quasi-two-dimensional verification problem derived from the following one-dimensional system:

!equation id=eq\:low-mach-system
\begin{aligned}
u\frac{\partial T}{\partial x} &= \frac{\alpha}{RePr}\frac{\partial^2T}{\partial x^2}+\dot{q}_0, \\
u\frac{\partial u}{\partial x} &= \frac{4\nu}{3Re}\frac{\partial^2u}{\partial x^2}-\frac{1}{\rho}\frac{\partial p_1}{\partial x}, \\
u\frac{\partial\rho}{\partial x} &= -\rho\frac{\partial u}{\partial x}, \\
\rho T &= 1.
\end{aligned}

Here, $T$ is the temperature, $u$ is the $x$-component of velocity, $\alpha$ is the thermal diffusivity, $Re$ is the Reynolds number, $Pr$ is the Prandtl number, $\dot{q}_0$ is the volumetric heat source, $\nu$ is the kinematic viscosity, $p_1$ is the hydrodynamic pressure, $\rho$ is the density, and $x$ is the spatial coordinate.

## Computational domain

The problem is solved on the domain $x\in[-1,1]$ and $y,z\in[0,1]$.
Periodic boundary conditions are applied in the $y$ and $z$ directions.

## Analytical solution

The volumetric heat source introduced by Tomboulides *et al.* [!citep](tomboulides1998quasi) is

!equation id=eq\:low-mach-heat-source
\dot{q}_0 = \frac{1}{\delta}\operatorname{sech}^2\left(\frac{x}{\delta}\right)\left[\frac{1}{2}+\frac{1}{\delta RePr}\tanh\left(\frac{x}{\delta}\right)\right].

The exact solution of the system is the smooth step profile

!equation id=eq\:low-mach-exact-solution
u(x)=T(x)=\frac{1}{2}\left[3+\tanh\left(\frac{x}{\delta}\right)\right],

where $\delta$ is a user-specified parameter that controls the sharpness of the solution profile.
Dirichlet boundary conditions are imposed at $x=-1$ and $x=1$ using the analytical solution.

## Verification results

The CI tests are performed using a polynomial order of seven and two CI modes.
Both CI modes verify the low-Mach solver in NekRS.
CI mode 2 additionally enables characteristic subcycling for the fluid and temperature solvers.
Errors are evaluated at $t=0.3$.
[fig:lowMach1] and [fig:lowMach2] present the volume-integrated error norms for the two CI modes.
The results demonstrate spectral convergence for the $x$-velocity, hydrodynamic pressure, and temperature fields, thereby verifying the accuracy of the low-Mach solver.

!media media/lowMach_1.png
       id=fig:lowMach1
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=Volume-integrated error norms for the *lowMach* case using CI mode 1.
       alt=Volume-integrated error norms for velocity, pressure, and temperature using CI mode 1

!media media/lowMach_2.png
       id=fig:lowMach2
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=Volume-integrated error norms for the *lowMach* case using CI mode 2.
       alt=Volume-integrated error norms for velocity, pressure, and temperature using CI mode 2
