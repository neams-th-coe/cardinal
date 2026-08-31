# Ethier Flow

This case is adopted from the nontrivial exact solution of the three-dimensional incompressible Navier--Stokes equations developed by Ethier and Steinman [!citep](ethier1994exact) for benchmarking incompressible [!ac](CFD) solvers.
The Ethier solution is also used to verify the passive scalar solvers in NekRS.
This is accomplished by selecting the $x$-component of the velocity field as the transported passive scalar and using the pressure term from the momentum equation as its source term, making the passive scalar transport equation identical to the $x$-momentum equation.
Because the analytical solution is known throughout the domain and for all times, both the incompressible flow and passive scalar solvers can be verified using only a few time steps, minimizing the computational cost of the tests.
Consequently, the *ethier* case verifies several NekRS capabilities through multiple different tests and solver modes.

## Computational domain

The problem is solved in the cubic domain

!equation id=eq\:ethier-domain
(x,y,z)\in[-1,1]^3.

Dirichlet boundary conditions obtained from the analytical solution are imposed for the velocity field.
Dirichlet and Neumann boundary conditions are imposed for the passive scalars $s_1$ and $s_2$, respectively.

## Analytical solution

The analytical velocity and pressure fields of Ethier and Steinman [!citep](ethier1994exact) are

!equation id=eq\:ethier-analytical-solution
\begin{aligned}
u &=
-a\left[
e^{ax}\sin(ay+dz)
+e^{az}\cos(ax+dy)
\right]e^{-d^2t}, \\
v &=
-a\left[
e^{ay}\sin(az+dx)
+e^{ax}\cos(ay+dz)
\right]e^{-d^2t}, \\
w &=
-a\left[
e^{az}\sin(ax+dy)
+e^{ay}\cos(az+dx)
\right]e^{-d^2t}, \\
p &=
-\frac{a^2}{2}
\left[
e^{2ax}+e^{2ay}+e^{2az}
+2\sin(ax+dy)\cos(az+dx)e^{a(y+z)}
\right. \\
&\qquad\left.
+2\sin(ay+dz)\cos(ax+dy)e^{a(z+x)}
+2\sin(az+dx)\cos(ay+dz)e^{a(x+y)}
\right]e^{-2d^2t},
\end{aligned}

where $a$ and $d$ are user-specified parameters, $\{x,y,z\}$ are the spatial coordinates, $\{u,v,w\}$ are the velocity components, $p$ is the pressure, and $t$ is time.
For the passive scalar verification tests, the transported analytical variable is the $x$-component of velocity, $u$.

## Verification criteria

The solution fields evaluated are

!equation id=eq\:ethier-solution-fields
\phi\in\{u,p,s_1,s_2\},

corresponding to the $x$-velocity, pressure, and two passive scalar fields.
For each evaluated field, the volume-integrated error norm is

!equation id=eq\:ethier-error-norm
\varepsilon_\phi
=
\left\lVert\phi-\phi_{\mathrm{exact}}\right\rVert_{L_2(\Omega)}
=
\left[
\int_\Omega
\left(\phi-\phi_{\mathrm{exact}}\right)^2
\,d\Omega
\right]^{1/2}.

The routine regression tests use polynomial order $N=9$ for every qualifying solver mode. The figures below are generated from separate $p$-refinement studies and are not part of the routine regression tests; they demonstrate spectral convergence as the polynomial order increases and provide additional evidence of the accuracy and consistency of the flow and passive scalar solvers.
The tests also monitor the numbers of iterations required for convergence of the velocity, pressure, and passive scalar solvers.
The specific NekRS capabilities verified by each test are described below.
The tests are organized into different solver modes as follows:

The qualifying postprocessor is evaluated at the final time and requires every enabled error norm to be below its mode-specific tolerance, or below $10^{-11}$ when that fallback criterion is satisfied.
The tolerances for the tested solver modes are summarized in [tab:ethier-pass-criteria].
Solver mode 13 is the scalar-only case, and solver mode 12 intentionally disables the temperature solve.

!table id=tab:ethier-pass-criteria caption=Pass criteria for the tested Ethier solver modes.
| Solver mode | Velocity error | Pressure error | Temperature error | Passive-scalar error |
| --- | ---: | ---: | ---: | ---: |
| 2, 3, 11, 14 | $4.78\times10^{-8}$ | $1.08\times10^{-7}$ | $9.48\times10^{-10}$ | $1.07\times10^{-9}$ |
| 4 | $5.90\times10^{-8}$ | $9.91\times10^{-8}$ | $2.09\times10^{-9}$ | $2.63\times10^{-9}$ |
| 5 | $2.21\times10^{-5}$ | $5.89\times10^{-5}$ | $1.04\times10^{-5}$ | $1.32\times10^{-5}$ |
| 6 | $1.17\times10^{-5}$ | $4.67\times10^{-5}$ | $5.20\times10^{-6}$ | $6.14\times10^{-6}$ |
| 7 | $2.51\times10^{-8}$ | $1.93\times10^{-6}$ | $2.90\times10^{-11}$ | $3.08\times10^{-11}$ |
| 8 | $8.68\times10^{-6}$ | $9.93\times10^{-3}$ | $7.29\times10^{-6}$ | $7.96\times10^{-6}$ |
| 9, 10 | $4.79\times10^{-8}$ | $1.08\times10^{-7}$ | $8.77\times10^{-10}$ | $1.02\times10^{-9}$ |
| 12 | $4.78\times10^{-8}$ | $1.08\times10^{-7}$ | -- | $1.07\times10^{-9}$ |
| 13 | -- | -- | -- | $1.53\times10^{-5}$ |

## Mode 2

This solver mode verifies:

- Incompressible Navier--Stokes and passive scalar solvers.
- Block velocity solver.
- Characteristic subcycling.

The regression error is evaluated at $t=0.06$. [fig:ethier-2] shows the corresponding $p$-refinement study.

!media media/ethier_2.png
       id=fig:ethier-2
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=Spectral convergence under p refinement for the *ethier* case using solver mode 2; the routine regression test uses polynomial order nine.
       alt=Spectral convergence of the Ethier solution errors using solver mode 2

## Mode 3

This solver mode verifies:

- Velocity and pressure projection.
- SEMFEM (low-order finite element) pressure preconditioner.

The regression error is evaluated at $t=0.06$. [fig:ethier-3] shows the corresponding $p$-refinement study.

!media media/ethier_3.png
       id=fig:ethier-3
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=Spectral convergence under p refinement for the *ethier* case using solver mode 3; the routine regression test uses polynomial order nine.
       alt=Spectral convergence of the Ethier solution errors using solver mode 3

## Mode 4

This solver mode verifies:

- Incompressible Navier--Stokes and passive scalar solvers.
- Block velocity solver.
- Characteristic subcycling.
- Velocity and pressure projection.

The regression error is evaluated at $t=0.2$. [fig:ethier-4] shows the corresponding $p$-refinement study.

!media media/ethier_4.png
       id=fig:ethier-4
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=Spectral convergence under p refinement for the *ethier* case using solver mode 4; the routine regression test uses polynomial order nine.
       alt=Spectral convergence of the Ethier solution errors using solver mode 4

## Mode 5

This solver mode verifies:

- Moving-mesh formulation.
- Block velocity solver.

The regression error is evaluated at $t=0.2$. [fig:ethier-5] shows the corresponding $p$-refinement study.

!media media/ethier_5.png
       id=fig:ethier-5
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=Spectral convergence under p refinement for the *ethier* case using solver mode 5; the routine regression test uses polynomial order nine.
       alt=Spectral convergence of the Ethier solution errors using solver mode 5

## Mode 6

This solver mode verifies:

- Moving-mesh formulation.
- Block velocity solver.
- Characteristic subcycling.

The regression error is evaluated at $t=0.2$. [fig:ethier-6] shows the corresponding $p$-refinement study.

!media media/ethier_6.png
       id=fig:ethier-6
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=Spectral convergence under p refinement for the *ethier* case using solver mode 6; the routine regression test uses polynomial order nine.
       alt=Spectral convergence of the Ethier solution errors using solver mode 6

## Mode 7

This solver mode verifies:

- Velocity and pressure projection.
- Jacobi pressure preconditioner.

The regression error is evaluated at $t=0.012$. [fig:ethier-7] shows the corresponding $p$-refinement study.

!media media/ethier_7.png
       id=fig:ethier-7
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=Spectral convergence under p refinement for the *ethier* case using solver mode 7; the routine regression test uses polynomial order nine.
       alt=Spectral convergence of the Ethier solution errors using solver mode 7

## Mode 8

This solver mode verifies:

- Pressure projection.
- Adaptive time stepping.
- Compliance of the final CFL number with its specified target.
- The expected number of time steps.

The regression error is evaluated at $t=0.2$. [fig:ethier-8] shows the corresponding $p$-refinement study.

!media media/ethier_8.png
       id=fig:ethier-8
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=Spectral convergence under p refinement for the *ethier* case using solver mode 8; the routine regression test uses polynomial order nine.
       alt=Spectral convergence of the Ethier solution errors using solver mode 8

## Mode 9

This solver mode verifies:

- Convective advection formulation without dealiasing.
- Block velocity solver.
- Characteristic subcycling.
- Velocity and pressure projection.

The regression error is evaluated at $t=0.06$. [fig:ethier-9] shows the corresponding $p$-refinement study.

!media media/ethier_9.png
       id=fig:ethier-9
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=Spectral convergence under p refinement for the *ethier* case using solver mode 9; the routine regression test uses polynomial order nine.
       alt=Spectral convergence of the Ethier solution errors using solver mode 9

## Mode 10

This solver mode verifies:

- Convective advection formulation without dealiasing.
- Block velocity solver.
- Velocity and pressure projection.

The regression error is evaluated at $t=0.06$. [fig:ethier-10] shows the corresponding $p$-refinement study.

!media media/ethier_10.png
       id=fig:ethier-10
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=Spectral convergence under p refinement for the *ethier* case using solver mode 10; the routine regression test uses polynomial order nine.
       alt=Spectral convergence of the Ethier solution errors using solver mode 10

## Mode 11

This solver mode verifies:

- Chebyshev-accelerated damped-Jacobi pressure multigrid smoother.
- Block velocity solver.
- Characteristic subcycling.
- Pressure projection.

The regression error is evaluated at $t=0.06$. [fig:ethier-11] shows the corresponding $p$-refinement study.

!media media/ethier_11.png
       id=fig:ethier-11
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=Spectral convergence under p refinement for the *ethier* case using solver mode 11; the routine regression test uses polynomial order nine.
       alt=Spectral convergence of the Ethier solution errors using solver mode 11

## Mode 12

This solver mode verifies that passive scalar 0 is disabled while passive scalar 1 continues to be solved correctly.
The regression error is evaluated at $t=0.06$. [fig:ethier-12] shows the corresponding $p$-refinement study.

!media media/ethier_12.png
       id=fig:ethier-12
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=Spectral convergence under p refinement for the *ethier* case using solver mode 12; the routine regression test uses polynomial order nine.
       alt=Spectral convergence of the Ethier solution errors using solver mode 12

## Mode 14

This solver mode verifies:

- Additive overlapping Schwarz pressure multigrid smoother.
- Block velocity solver.
- Characteristic subcycling.
- Pressure projection.

The regression error is evaluated at $t=0.06$. [fig:ethier-14] shows the corresponding $p$-refinement study.

!media media/ethier_14.png
       id=fig:ethier-14
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=Spectral convergence under p refinement for the *ethier* case using solver mode 14; the routine regression test uses polynomial order nine.
       alt=Spectral convergence of the Ethier solution errors using solver mode 14
