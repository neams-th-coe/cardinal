# Moving Cylinder (Low-Mach)

The *mv_cyl* case verifies the low-Mach and moving-mesh capabilities in NekRS using a moving piston in a quasi-two-dimensional domain.
The piston is located at $y=-L_y$, while a stationary wall is located at $y=0$.
The domain has width $L_x$, and symmetry boundary conditions are imposed at $x=\{0,L_x\}$.

## Governing equations

Under the ideal-gas assumption, the nondimensional low-Mach governing equations are

!equation id=eq\:mv-cyl-governing-equations
\begin{aligned}
\nabla\cdot\vec{v} &= \frac{1}{T}\frac{DT}{Dt}-\frac{1}{p_t}\frac{dp_t}{dt}=Q, \\
\rho\left(\frac{\partial\vec{v}}{\partial t}+\vec{v}\cdot\nabla\vec{v}\right) &= -\nabla p_1+\frac{1}{Re}\nabla\cdot\left(2\boldsymbol{\underline{S}}-\frac{2}{3}Q\boldsymbol{\underline{I}}\right), \\
\rho c_p\frac{DT}{Dt} &= \frac{1}{Pe}\nabla\cdot\nabla T+\frac{\gamma-1}{\gamma}\frac{dp_t}{dt}, \\
p_t &= \rho T.
\end{aligned}

Here, $Q$ is the thermal divergence, $p_t$ is the thermodynamic pressure, $p_1$ is the hydrodynamic pressure, $\gamma$ is the ratio of specific heats, $\boldsymbol{\underline{S}}=\frac{1}{2}\left(\nabla\vec{v}+\nabla\vec{v}^{\,T}\right)$ is the strain-rate tensor, and $\boldsymbol{\underline{I}}$ is the identity tensor.
The Reynolds and Péclet numbers are denoted by $Re$ and $Pe$, respectively.

## Analytical solution

The piston is prescribed a sinusoidal velocity,

!equation id=eq\:mv-cyl-piston-velocity
v_p(t)=A\sin(\omega t),

where $A$ is the piston velocity amplitude.
The flow remains isentropic, and the analytical solution is

!equation id=eq\:mv-cyl-analytical-solution
\begin{aligned}
V(t) &= V_0+A_pA\frac{\cos(\omega t)-1}{\omega}, \\
p_t(t) &= p_0\left(\frac{V_0}{V(t)}\right)^\gamma, \\
\frac{dp_t}{dt} &= \gamma p_0A_p\frac{V_0}{V(t)^{\gamma+1}}v_p(t), \\
y_p(t) &= -\frac{1}{2}\left[1+\cos(\omega t)\right], \\
Q &= \left(\frac{\gamma-1}{\gamma}-1\right)\frac{1}{p_t(t)}\frac{dp_t}{dt}, \\
\overline{T}(t) &= p_t(t)^{\frac{\gamma-1}{\gamma}}.
\end{aligned}

Here, $y_p(t)$ is the piston location, $V_0$ is the initial domain volume, $V(t)$ is the instantaneous domain volume, $p_0$ is the initial thermodynamic pressure, $A_p$ is the piston surface area, and $\overline{T}(t)$ is the spatially averaged temperature.

## Verification criterion

The tests are qualified by evaluating the absolute errors in $\{V(t),dV(t)/dt,p_t(t),dp_t/dt,y_p(t),\overline{T}(t)\}$ at a specified time.
Because these quantities are spatially uniform, the case also verifies the expected temporal convergence rate.

For the various tests, the mesh velocity is either prescribed analytically as,

!equation id=eq\:mv-cyl-mesh-velocity
v_m(y,t)=v_p(t)\frac{y-y_{\max}(t)}{y_{\min}(t)-y_{\max}(t)}.

or the mesh motion is computed using the elasticity solver with the piston velocity prescribed as a Dirichlet boundary condition.
The resulting temporal convergence is presented below.
The tests are organized into various solver modes, as follows:

## Mode 1

This solver mode tests:

- Low-Mach solver.
- Passive scalar solver.
- Prescribed moving mesh.

Errors are computed at $t=0.1$ and are shown in [fig:mv-cyl-1].

!media media/mv_cyl_1.png
       id=fig:mv-cyl-1
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=Temporal convergence for the *mv_cyl* case using solver mode 1.
       alt=Temporal convergence results for the moving-cylinder case using solver mode 1

## Mode 2

This solver mode tests:

- All capabilities exercised in solver mode 1.
- Characteristic subcycling.

Errors are computed at $t=0.1$ and are shown in [fig:mv-cyl-2].

!media media/mv_cyl_2.png
       id=fig:mv-cyl-2
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=Temporal convergence for the *mv_cyl* case using solver mode 2.
       alt=Temporal convergence results for the moving-cylinder case using solver mode 2

## Mode 3

This solver mode tests:

- All capabilities exercised in solver mode 2.
- Elasticity mesh solver.
- Mesh projection.

Errors are computed at $t=0.1$ and are shown in [fig:mv-cyl-3].

!media media/mv_cyl_3.png
       id=fig:mv-cyl-3
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=Temporal convergence for the *mv_cyl* case using solver mode 3.
       alt=Temporal convergence results for the moving-cylinder case using solver mode 3

## Mode 5

This solver mode verifies the capabilities of solver mode 1 on an unaligned mesh.
Errors are computed at $t=0.1$ and are shown in [fig:mv-cyl-5].

!media media/mv_cyl_5.png
       id=fig:mv-cyl-5
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=Temporal convergence for the *mv_cyl* case using solver mode 5.
       alt=Temporal convergence results for the moving-cylinder case using solver mode 5

## Mode 6

This solver mode verifies the capabilities of solver mode 3 on an unaligned mesh.
Errors are computed at $t=0.1$ and are shown in [fig:mv-cyl-6].

!media media/mv_cyl_6.png
       id=fig:mv-cyl-6
       style=width:60%;margin-left:auto;margin-right:auto;
       caption=Temporal convergence for the *mv_cyl* case using solver mode 6.
       alt=Temporal convergence results for the moving-cylinder case using solver mode 6
