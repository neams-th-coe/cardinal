# Turbulent Pipe Flow (LES)

This case verifies the [!ac](LES) capability in NekRS using fully developed turbulent pipe flow.

## Flow parameters

The pipe has a nondimensional diameter $D=1$ and a bulk Reynolds number $Re_b=19000$, defined as

!equation id=eq\:turb-pipe-reynolds
Re_b = \frac{U_b D}{\nu},

where $U_b$ is the bulk velocity and $\nu$ is the kinematic viscosity.
The corresponding friction Reynolds number is approximately $Re_\tau=550$.

## Subgrid-scale model

Subgrid-scale modeling is performed using the high-pass filtering approach of Stolz *et al.* [!citep](stolz2005high).
A cutoff wavenumber of $N-1$ is used to construct the low-pass filter, and the filter strength is set to 10.

## Verification criterion

The simulation is qualified by computing the time-averaged velocity field and evaluating the friction velocity,

!equation id=eq\:turb-pipe-friction-velocity
u_\tau = \sqrt{\frac{\tau_w}{\rho}},

where the wall shear stress is

!equation id=eq\:turb-pipe-wall-shear
\tau_w = 2\mu\left|\underline{S}\cdot\vec{n}\right|_w,

and the mean strain-rate tensor is

!equation id=eq\:turb-pipe-strain-rate
\underline{S} = \frac{1}{2}\left(\nabla\vec{v}+\nabla\vec{v}^{\,T}\right).

Here, $\vec{v}$ is the time-averaged velocity, $\tau_w$ is the computed wall shear stress, $\rho$ is the fluid density, $\mu$ is the dynamic viscosity, $\underline{S}$ is the strain-rate tensor, and $\vec{n}$ is the outward unit normal vector at the wall.
The computed friction velocity is compared with the direct numerical simulation (DNS) reference of El Khoury *et al.* [!citep](el2013direct) using

!equation id=eq\:turb-pipe-error
\mathrm{err} = \frac{\left|u_\tau-u_{\tau,\mathrm{DNS}}\right|}{u_{\tau,\mathrm{DNS}}},

where

!equation id=eq\:turb-pipe-dns-friction-velocity
u_{\tau,\mathrm{DNS}} = 5.79\times10^{-2}.

The case passes when the relative friction-velocity error is below $2.00\times10^{-2}$ ($2\%$).
