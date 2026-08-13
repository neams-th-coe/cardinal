# Ethier Flow

This case is adopted from the nontrivial exact solution of the three-dimensional incompressible Navier--Stokes equations developed by Ethier and Steinman [!citep](ethier1994exact) for benchmarking incompressible CFD solvers.
The Ethier solution is also used to verify the passive scalar solvers in NekRS.
This is accomplished by selecting the $x$-component of the velocity field as the transported passive scalar and using the pressure term from the momentum equation as its source term, making the passive scalar transport equation identical to the $x$-momentum equation.
Because the analytical solution is known throughout the domain and for all times, both the incompressible flow and passive scalar solvers can be verified using only a few time steps, minimizing the computational cost of each regression test.
Consequently, the *ethier* case verifies several NekRS capabilities through multiple CI modes.

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

The solution fields evaluated by the CI tests are

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

The error norms are evaluated using multiple polynomial orders $N$.
The results demonstrate spectral convergence as the polynomial order increases, confirming the accuracy and consistency of the flow and passive scalar solvers.
The CI tests also monitor the numbers of iterations required for convergence of the velocity, pressure, and passive scalar solvers.
The specific NekRS capabilities verified by each CI mode are described below.

## CI mode 2

This CI mode verifies:

- Incompressible Navier--Stokes and passive scalar solvers.
- Block velocity solver.
- Characteristic subcycling.

Errors are evaluated at $t=0.06$ and are shown in [fig:ethier-2].

!media media/ethier_2.png
       id=fig:ethier-2
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=$L_2$ error norms for the *ethier* case using CI mode 2.
       alt=Spectral convergence of the Ethier solution errors using CI mode 2

## CI mode 3

This CI mode verifies:

- Velocity and pressure projection.
- SEMFEM pressure preconditioner.

Errors are evaluated at $t=0.06$ and are shown in [fig:ethier-3].

!media media/ethier_3.png
       id=fig:ethier-3
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=$L_2$ error norms for the *ethier* case using CI mode 3.
       alt=Spectral convergence of the Ethier solution errors using CI mode 3

## CI mode 4

This CI mode verifies:

- Incompressible Navier--Stokes and passive scalar solvers.
- Block velocity solver.
- Characteristic subcycling.
- Velocity and pressure projection.

Errors are evaluated at $t=0.2$ and are shown in [fig:ethier-4].

!media media/ethier_4.png
       id=fig:ethier-4
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=$L_2$ error norms for the *ethier* case using CI mode 4.
       alt=Spectral convergence of the Ethier solution errors using CI mode 4

## CI mode 5

This CI mode verifies:

- Moving-mesh formulation.
- Block velocity solver.

Errors are evaluated at $t=0.2$ and are shown in [fig:ethier-5].

!media media/ethier_5.png
       id=fig:ethier-5
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=$L_2$ error norms for the *ethier* case using CI mode 5.
       alt=Spectral convergence of the Ethier solution errors using CI mode 5

## CI mode 6

This CI mode verifies:

- Moving-mesh formulation.
- Block velocity solver.
- Characteristic subcycling.

Errors are evaluated at $t=0.2$ and are shown in [fig:ethier-6].

!media media/ethier_6.png
       id=fig:ethier-6
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=$L_2$ error norms for the *ethier* case using CI mode 6.
       alt=Spectral convergence of the Ethier solution errors using CI mode 6

## CI mode 7

This CI mode verifies:

- Velocity and pressure projection.
- Jacobi pressure preconditioner.

Errors are evaluated at $t=0.012$ and are shown in [fig:ethier-7].

!media media/ethier_7.png
       id=fig:ethier-7
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=$L_2$ error norms for the *ethier* case using CI mode 7.
       alt=Spectral convergence of the Ethier solution errors using CI mode 7

## CI mode 8

This CI mode verifies:

- Pressure projection.
- Adaptive time stepping.
- Compliance of the final CFL number with its specified target.
- The expected number of time steps.

Errors are evaluated at $t=0.2$ and are shown in [fig:ethier-8].

!media media/ethier_8.png
       id=fig:ethier-8
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=$L_2$ error norms for the *ethier* case using CI mode 8.
       alt=Spectral convergence of the Ethier solution errors using CI mode 8

## CI mode 9

This CI mode verifies:

- Convective advection formulation without dealiasing.
- Block velocity solver.
- Characteristic subcycling.
- Velocity and pressure projection.

Errors are evaluated at $t=0.06$ and are shown in [fig:ethier-9].

!media media/ethier_9.png
       id=fig:ethier-9
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=$L_2$ error norms for the *ethier* case using CI mode 9.
       alt=Spectral convergence of the Ethier solution errors using CI mode 9

## CI mode 10

This CI mode verifies:

- Convective advection formulation without dealiasing.
- Block velocity solver.
- Velocity and pressure projection.

Errors are evaluated at $t=0.06$ and are shown in [fig:ethier-10].

!media media/ethier_10.png
       id=fig:ethier-10
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=$L_2$ error norms for the *ethier* case using CI mode 10.
       alt=Spectral convergence of the Ethier solution errors using CI mode 10

## CI mode 11

This CI mode verifies:

- Chebyshev-accelerated damped-Jacobi pressure multigrid smoother.
- Block velocity solver.
- Characteristic subcycling.
- Pressure projection.

Errors are evaluated at $t=0.06$ and are shown in [fig:ethier-11].

!media media/ethier_11.png
       id=fig:ethier-11
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=$L_2$ error norms for the *ethier* case using CI mode 11.
       alt=Spectral convergence of the Ethier solution errors using CI mode 11

## CI mode 12

This CI mode verifies that passive scalar 0 is disabled while passive scalar 1 continues to be solved correctly.
Errors are evaluated at $t=0.06$ and are shown in [fig:ethier-12].

!media media/ethier_12.png
       id=fig:ethier-12
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=$L_2$ error norms for the *ethier* case using CI mode 12.
       alt=Spectral convergence of the Ethier solution errors using CI mode 12

## CI mode 14

This CI mode verifies:

- Additive overlapping Schwarz pressure multigrid smoother.
- Block velocity solver.
- Characteristic subcycling.
- Pressure projection.

Errors are evaluated at $t=0.06$ and are shown in [fig:ethier-14].

!media media/ethier_14.png
       id=fig:ethier-14
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=$L_2$ error norms for the *ethier* case using CI mode 14.
       alt=Spectral convergence of the Ethier solution errors using CI mode 14
