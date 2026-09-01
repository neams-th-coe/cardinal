# Backward-Facing Step

The backward-facing step experiment of Driver and Seegmiller [!citep](driver1985features) is a standard validation benchmark for [!ac](RANS) turbulence models.
The predicted reattachment length downstream of the step is commonly used as a primary metric for assessing turbulence-model performance.
The *backward-facing step* case verifies the $k$-$\tau$ [!ac](RANS) model in NekRS by comparing the predicted skin-friction coefficient along the downstream wall with the experimental measurements of Driver and Seegmiller [!citep](driver1985features).

## Computational domain

The step is located at the origin, and the inlet and outlet boundaries are located at $x/H=-4$ and $x/H=40$, respectively, where $H$ is the step height.
The upstream and downstream channel heights are $8H$ and $9H$, respectively.
The Reynolds number based on the inlet velocity $U$ and the step height is $Re=37{,}425$.
The simulation is advanced to a quasi-steady state before the validation metrics are evaluated.
[fig:bfs1] shows the resulting streamwise velocity and turbulent kinetic energy fields.

!media media/bfs-contour.png
       id=fig:bfs1
       style=width:80%;margin-left:auto;margin-right:auto;
       caption=Quasi-steady streamwise velocity and turbulent kinetic energy contours.
       alt=Streamwise velocity and turbulent kinetic energy contours for the backward-facing step

## Verification criterion

The test is qualified by comparing the computed skin-friction coefficient along the downstream wall, $x>0$, with the experimental measurements of Driver and Seegmiller [!citep](driver1985features).
The skin-friction coefficient is defined as

!equation id=eq\:bfs-skin-friction
C_f = \frac{2\tau_w}{\rho_0 U^2},

where $\tau_w$ is the wall shear stress, $\rho_0$ is the reference density, and $U$ is the reference inlet velocity.
The test evaluates the area-normalized absolute error in the skin-friction coefficient along the downstream-wall boundary,

!equation id=eq\:bfs-error
\varepsilon_{C_f}
=
\frac{
\displaystyle\int_{\Gamma_w}\left|C_f-C_f^{\mathrm{exp}}\right|\,dA
}{
\displaystyle\int_{\Gamma_w}dA
}.

[fig:bfs2] compares the computed skin-friction coefficient profile with the experimental measurements.
A polynomial fit to the experimental data is used to evaluate the error norm.
The case passes when $\varepsilon_{C_f}<1.38\times10^{-3}$.

!media media/bfs-cf.png
       id=fig:bfs2
       style=width:70%;margin-left:auto;margin-right:auto;
       caption=Comparison of the computed skin-friction coefficient with the experimental measurements of Driver and Seegmiller [!citep](driver1985features).
       alt=Computed and experimental skin-friction coefficient profiles downstream of the backward-facing step
