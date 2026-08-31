# Stokes Flow

NekRS provides the option to solve unsteady Stokes flow with constant or variable viscosity.
The *channel* case verifies the Stokes flow solver using the [!ac](MMS).
The problem is solved in a quasi-two-dimensional square domain with edge length 2 and an arbitrary user-specified orientation angle $\alpha$.

## Governing equations

The nondimensional unsteady Stokes equations for an incompressible fluid with spatially varying kinematic viscosity are

!equation id=eq\:channel-stokes-equations
\begin{aligned}
\frac{\partial\boldsymbol{u}}{\partial t}
&=
-\nabla p
+\nabla\cdot\left[\nu(x,y)\left(\nabla\boldsymbol{u}+\nabla\boldsymbol{u}^{\,T}\right)\right]
+\boldsymbol{f}, \\
\nabla\cdot\boldsymbol{u}
&=0,
\end{aligned}

where $\boldsymbol{u}$ is the velocity vector, $p$ is the hydrodynamic pressure, $\nu$ is the kinematic viscosity, and $\boldsymbol{f}$ is a prescribed forcing function.
The nonlinear advection term is omitted from the momentum equation, distinguishing Stokes flow from the full incompressible Navier-Stokes equations.

## Manufactured solution

The manufactured steady-state velocity solution is

!equation id=eq\:channel-manufactured-velocity
\begin{aligned}
u' &= U_0\cos\left[\pi(x+0.5)\right]\sin\left[\pi(2y+0.5)\right], \\
v' &= -\frac{U_0}{2}\sin\left[\pi(x+0.5)\right]\cos\left[\pi(2y+0.5)\right], \\
u(x,y) &= u'\cos(\alpha)-v'\sin(\alpha), \\
v(x,y) &= u'\sin(\alpha)+v'\cos(\alpha).
\end{aligned}

The manufactured viscosity field is

!equation id=eq\:channel-manufactured-viscosity
\nu(x,y)=\frac{1}{Re}(1+ay),

where $Re$ is the Reynolds number, $U_0$ is a user-specified velocity scale, $a$ is a user-specified viscosity scaling parameter, and $u$ and $v$ are the velocity components in the $x$ and $y$ directions, respectively.
The spatial variation in viscosity introduces an additional contribution to the viscous operator through $\nabla\nu$.
The corresponding manufactured forcing function is

!equation id=eq\:channel-forcing
\boldsymbol{f}
=
-\nabla\nu\cdot
\left(\nabla\boldsymbol{u}+\nabla\boldsymbol{u}^{\,T}\right).

## Verification criteria

The simulations are performed using a polynomial order of seven and two geometrical configurations.
The first configuration uses the original geometry, while the second rotates the geometry by $45^\circ$.
Errors are evaluated at $t=0.1$; because the initial condition is the exact solution, this time is short enough for the test to run quickly, but long enough such that any code changes or inconsistencies in the implementation would manifest as errors.
The volume-integrated velocity error is defined as

!equation id=eq\:channel-velocity-error
\varepsilon_{\boldsymbol{u}}
=
\left\lVert\boldsymbol{u}-\boldsymbol{u}_{\mathrm{exact}}\right\rVert_{L_2(\Omega)}
=
\left[
\int_\Omega
\left|\boldsymbol{u}-\boldsymbol{u}_{\mathrm{exact}}\right|^2
\,d\Omega
\right]^{1/2}.

For both geometries, the final-time pass criterion requires the $L_2$ errors in both velocity components to be below $9.27\times10^{-7}$.

[fig:channel1] and [fig:channel2] present separate $p$-refinement studies for the two configurations. These refinement sweeps are not part of the routine regression tests, which use polynomial order $N=7$; the figures showcase spectral convergence of the velocity solution and provide additional evidence of the Stokes flow solver's accuracy.

!media media/channel_1.png
       id=fig:channel1
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=Spectral convergence under p refinement for the original-geometry *channel* case; the routine regression test uses polynomial order seven.
       alt=Volume-integrated velocity error norms for the Stokes flow case using original geometry

!media media/channel_2.png
       id=fig:channel2
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=Spectral convergence under p refinement for the rotated-geometry *channel* case; the routine regression test uses polynomial order seven.
       alt=Volume-integrated velocity error norms for the Stokes flow case using rotated geometry
