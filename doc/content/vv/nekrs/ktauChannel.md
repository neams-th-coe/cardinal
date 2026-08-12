# Turbulent Channel Flow (RANS)

This case verifies the standard $k$-$\tau$ Reynolds-averaged Navier–Stokes (RANS) turbulence model [@tomboulides2025robust] using fully developed turbulent flow in an infinite half-channel.
Two CI modes verify the nondimensional and dimensional formulations of the same flow.

## Flow parameters

The bulk Reynolds number is $Re = 43500$ and is defined as

!equation id=eq:ktau-channel-reynolds
Re = \frac{\rho_0 U_b L}{\mu_0},

where $L$, $U_b$, $\rho_0$, and $\mu_0$ are the dimensional reference length, bulk velocity, density, and dynamic viscosity, respectively.
The corresponding friction Reynolds number, based on the friction velocity $u_\tau$, is approximately 2000 and is defined as

!equation id=eq:ktau-channel-friction-reynolds
Re_\tau = \frac{\rho_0 u_\tau L}{\mu_0},

where

!equation id=eq:ktau-channel-friction-velocity
u_\tau = \sqrt{\frac{\tau_w}{\rho_0}}.

Here, $\tau_w$ is the computed wall shear stress.

## Nondimensional formulation

CI mode 1 uses the nondimensional formulation with

!equation id=eq:ktau-channel-nondimensional-scales
L^* = 1, \qquad U_b^* = 1, \qquad \rho^* = 1, \qquad \mu^* = \frac{1}{Re},

where the superscript $*$ denotes a nondimensional quantity.
The computed nondimensional friction velocity is compared with the direct numerical simulation (DNS) reference of Lee *et al.* [@lee2015direct],

!equation id=eq:ktau-channel-dns-friction-velocity
u_{\tau,\mathrm{DNS}}^* = 4.58794 \times 10^{-2}.

## Dimensional formulation

CI mode 2 represents the same flow using the dimensional reference scales $L$, $U_b$, $\rho_0$, and $\mu_0$.
The dimensional dynamic viscosity is selected to preserve the bulk Reynolds number,

!equation id=eq:ktau-channel-dimensional-viscosity
\mu_0 = \frac{\rho_0 U_b L}{Re}.

The corresponding dimensional DNS friction velocity is

!equation id=eq:ktau-channel-dimensional-friction-velocity
u_{\tau,\mathrm{DNS}} = U_b u_{\tau,\mathrm{DNS}}^*.

## Verification criterion

For both CI modes, the relative error is evaluated using

!equation id=eq:ktau-channel-relative-error
\mathrm{err} = \frac{\left|u_\tau-u_{\tau,\mathrm{DNS}}\right|}{u_{\tau,\mathrm{DNS}}}.

The nondimensional quantities $u_\tau^*$ and $u_{\tau,\mathrm{DNS}}^*$ are used for CI mode 1.
This comparison verifies that dimensional scaling preserves the predicted friction velocity and the nondimensional flow behavior.
