# Laminar natural convection in a 2-D square cavity

This test case models two-dimensional laminar flow of a Boussinesq fluid in an upright square cavity. 
The computational domain follows the benchmark setup of [Davis and Jones (1983)](https://onlinelibrary.wiley.com/doi/abs/10.1002/fld.1650030304). The objective is to compute the velocity and temperature fields for various Rayleigh numbers. Note that this benchmark does not represent an analytic solution, but rather a numerical benchmark.

## Computational Domain

The domain is a square cavity of side length $L$. The left wall is held at temperature $T_1$, while the right wall is at $T_2$. The top and bottom walls are insulated. The fluid has a Prandtl number of 0.71. Computations are performed for Rayleigh numbers of $10^3$, $10^4$, $10^5$, and $10^6$. The benchmark defines the following non-dimensional quantities:

!equation
x = \frac{\tilde{x}}{L}, 
\quad z = \frac{\tilde{z}}{L},
\quad u = \frac{\tilde{u}L}{\alpha},
\quad w = \frac{\tilde{w}L}{\alpha},
\quad T = \frac{\tilde{T} - T_2}{T_1 - T_2},
\quad Pr = \frac{\nu}{\alpha},
\quad Ra = \frac{\beta g \Delta T L^3}{\alpha \nu}

Where $\alpha$ is the thermal diffusivity of the fluid and $\nu$ is the kinematic viscosity. The parameters of the computational domain are summarized in [table1].

!table id=table1 caption=Properties of the computational domain.
| Parameter | Value |
| :- | :- |
| x | $0 \leq x \leq 1$ |
| z | $0 \leq z \leq 1$ |
| $T_1$ | $1$ |
| $T_2$ | $0$ |
| $Pr$ | $0.71$ |
| $Ra$ | $10^3, 10^4, 10^5, 10^6$ |
| $Pe$ | $1$
| $Re$ | $1.4085$ |
| $Ri$ | $0.71 \cdot Ra$ |

## Results

[fig_vel] shows the velocity field with streamlines, while [fig_temp] displays the temperature field along with temperature contours. To compare the results with the benchmark, the following quantities are reported in [table2]: the maximum horizontal velocity along the vertical centerline ($u_{max}$), the maximum vertical velocity along the horizontal centerline ($w_{max}$), and the average Nusselt number on the hot wall ($\overline{Nu}$). The simulation results are in excellent agreement with the benchmark.

!media natural_convection_velocity.png
  id=fig_vel
  caption=Velocity Field
  style=width:100%;margin-left:auto;margin-right:auto;halign:center

!media natural_convection_temp.png
  id=fig_temp
  caption=Temperature
  style=width:100%;margin-left:auto;margin-right:auto;halign:center

!table id=table2 caption=Comparison of Cardinal results with the benchmark
| Ra | $10^3$ |  |  | $10^4$ |  |  | $10^5$ |  |  | $10^6$ |
| :- | :- | :- | :- | :- | :- | :- | :- | :- | :- | :- | :- | :- | :- | :- | :- |
| - | Cardinal | Benchmark | Relative Error | Cardinal | Benchmark | Relative Error | Cardinal | Benchmark | Relative Error | Cardinal | Benchmark | Relative Error |
| $u_{max}$ | 3.649 | 3.649 | 0.0E+00 | 16.171 | 16.178 | 4.3E-04 | 34.66 | 34.73 | 2.0E-03 | 64.80 | 64.63 | 2.6E-03 |
| $w_{max}$ | 3.697 | 3.697 | 0.0E+00 | 19.627 | 19.617 | 5.1E-04 | 68.63 | 68.59 | 5.8E-04 | 220.56 | 219.36 | 5.4E-03 |
| $\overline{Nu}$ | 1.118 | 1.118 | 0.0E+00 | 2.244 | 2.243 | 4.5E-04 | 4.522 | 4.519 | 6.6E-04 | 8.825 | 8.800 | 2.8E-03 |   

A mesh sensitivity study was conducted to ensure that the mesh resolution was sufficient. [table3] presents the average Nusselt number computed using three different mesh sizes. Since the results obtained with the medium and fine meshes are identical, the medium mesh was selected for the test case. All results reported in [table2] were computed using this mesh. The medium mesh is shown in [fig_mesh]

!table id=table3 caption=Mesh sensitivity
| Mesh | Number of Elements | $\overline{Nu}$ |
| :- | :- | :- |
| Coarse | $377$ | 1.121 |
| Medium | $377 \times 4$ | 1.118 |
| Fine | $377 \times 16$ | 1.118  |

!media natural_convection_mesh.png
  id=fig_mesh
  caption=Computational mesh (1508 Elements).
  style=width:50%;margin-left:auto;margin-right:auto;halign:center