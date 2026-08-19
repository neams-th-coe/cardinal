# Conjugate Heat Transfer

NekRS provides a built-in [!ac](CHT) module for simulations involving conforming fluid and solid domains.
[!ac](CHT) can also be accomplished by coupling NekRS's flow solver with MOOSE via Cardinal, but here the standalone [!ac](CHT) solver in NekRS is tested.
The *conj_ht* case verifies the CHT module against an analytical solution.
The computational domain is illustrated in [fig:conj-ht-geometry].
The domain consists of a fluid channel of height $H$ bounded by solid plates of equal height $H_p$.
The fluid channel and solid plates have length $L$, and the domain is periodic in the transverse $z$ direction.

!media media/conj_ht_geometry.png
       id=fig:conj-ht-geometry
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=Geometry and boundary conditions for the *conj_ht* case.
       alt=Conjugate heat transfer geometry showing the fluid channel, solid plates, and boundary conditions

## Analytical solution

Fully developed hydrodynamic and thermal conditions are assumed.
The velocity field is prescribed using the Poiseuille flow solution,

!equation id=eq\:conj-ht-velocity
u(y)=Re\frac{dp}{dx}y(1-y).

A nondimensional uniform heat source, $\dot{q}$, is applied throughout both solid plates.
The analytical temperature solution is

!equation id=eq\:conj-ht-temperature
\begin{aligned}
T_F(x,y) &= -\dot{q}Pe\frac{H_p}{H}\left(y^4-2y^3+y\right)+\dot{q}\frac{H_p}{H}\left(2x+\frac{17}{70}Pe\right), \\
T_I(x,y) &= -\dot{q}Pe\frac{1}{k_r}\left(\frac{y^2}{2}+y\frac{H_p}{H}\right)+\dot{q}\frac{H_p}{H}\left(2x+\frac{17}{70}Pe\right), \\
T_S(x,y) &= -\dot{q}Pe\frac{1}{k_r}\left[\frac{y^2}{2}-y\left(1+\frac{H_p}{H}\right)+\left(\frac{1}{2}+\frac{H_p}{H}\right)\right]+\dot{q}\frac{H_p}{H}\left(2x+\frac{17}{70}Pe\right).
\end{aligned}

Here, $T_F$ denotes the fluid temperature, while $T_I$ and $T_S$ denote the temperatures in the lower and upper solid plates, respectively.
The conductivity ratio is defined as $k_r=k_s/k_f$, where $k_s$ and $k_f$ are the solid and fluid thermal conductivities.
The Reynolds and Péclet numbers are defined as

!equation id=eq\:conj-ht-dimensionless-numbers
\begin{aligned}
Re &= \frac{\rho_fU_0H}{\mu_f}, \\
Pe &= \frac{\rho_fU_0Hc_{pf}}{k_f}.
\end{aligned}

## Case parameters

The nondimensional parameters used in the *conj_ht* test are summarized in [tab:conj-ht-setup].

!table id=tab:conj-ht-setup caption=Case properties and simulation parameters.
| Parameter | Variable | Value |
| --- | --- | ---: |
| Nondimensional channel height | $H$ | 1 |
| Nondimensional channel length | $L$ | 8 |
| Nondimensional plate height | $H_p$ | 0.5 |
| Reynolds number | $Re$ | 500 |
| Péclet number | $Pe$ | 1000 |
| Heat source | $\dot{q}$ | 1 |
| Fluid density | $\rho_f$ | 1 |
| Fluid volumetric heat capacity | $\rho_fc_{pf}$ | 1 |
| Solid volumetric heat capacity | $\rho_sc_{ps}$ | 0.1 |
| Solid-to-fluid conductivity ratio | $k_r$ | 10 |
| Pressure gradient | $\dfrac{\partial p}{\partial x}$ | 0.012 |

## Boundary conditions

Dirichlet boundary conditions are imposed at $x=0$ for both the velocity and temperature fields.
An outflow boundary condition is imposed at $x=L$ for the velocity field.
A Neumann boundary condition derived from the analytical solution is imposed for temperature,

!equation id=eq\:conj-ht-outlet-temperature
\left.\frac{k}{Pe}\nabla T\cdot\vec{n}\right|_{x=L}
=
\left.\frac{k}{Pe}\frac{\partial T}{\partial x}\right|_{x=L}
=
2\dot{q}\frac{H_p}{H}\frac{k}{Pe},

where $\vec{n}$ is the outward unit normal vector.
The conductivity is $k=1$ in the fluid and $k=k_r$ in the solid.
Insulated boundary conditions are imposed on the outer surfaces of both solid plates.

## Verification criteria

Verification of this case is performed by evaluating $L_2$ error norms of the streamwise velocity and temperature fields at steady state.
NekRS offers a characteristics based integration scheme, [!ac](OIFS), for the advection operator, which allows NekRS to run with [!ac](CFL) > 1 [!citep](fischer2003implementation), depending on the number of steps chosen by the user for advection subcycling. 
This simulation is run with two solver modes, with and without advection subcycling enabled.
The velocity error is defined as

!equation id=eq\:conj-ht-velocity-error
\varepsilon_u
=
\left\lVert u_x-u_{\mathrm{exact}}\right\rVert_{L_2(\Omega)}
=
\left[\int_\Omega\left(u_x-u_{\mathrm{exact}}\right)^2\,d\Omega\right]^{1/2},

and the temperature error is defined as

!equation id=eq\:conj-ht-temperature-error
\varepsilon_T
=
\left\lVert T-T_{\mathrm{exact}}\right\rVert_{L_2(\Omega)}
=
\left[\int_\Omega\left(T-T_{\mathrm{exact}}\right)^2\,d\Omega\right]^{1/2}.

Here, $u_{\mathrm{exact}}$ is the prescribed Poiseuille velocity profile and $T_{\mathrm{exact}}$ is the piecewise analytical temperature solution in the fluid and solid domains.
The simulations are considered to match the analytic solution when both error tolerances are satisfied for each solver mode,

!equation id=eq\:conj-ht-pass-criterion
\varepsilon_u < 7.0\times10^{-8}
\qquad\text{and}\qquad
\varepsilon_T < 1.2\times10^{-5}.
