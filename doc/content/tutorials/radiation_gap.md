The gap region is unmeshed, and a quadrature-based
thermal contact model is applied based on the sum of thermal conduction and thermal radiation
(across a transparent medium).
For a paired set of boundaries,
each quadrature point on boundary $A$ is paired with the nearest quadrature point on boundary $B$.
Then, the sum of the radiation and conduction heat fluxes imposed between
quadrature point pairs is

\begin{equation}
\label{eq:3}
q^{''}=\sigma\frac{T^4-T_{gap}^4}{\frac{1}{\sigma_A}+\frac{1}{\sigma_B}-1}+\frac{T-T_{gap}}{r_{th}}
\end{equation}

where $\sigma$ is the Stefan-Boltzmann constant, $T$ is the temperature at a quadrature
point, $T_{gap}$ is the temperature of the nearest quadrature point across the gap,
$\sigma_A$ and $\sigma_B$ are emissivities of boundaries $A$ and $B$, respectively, and
$r_{th}$ is the conduction resistance.
For cylindrical geometries, the conduction
resistance is given as

\begin{equation}
\label{eq:4}
r_{th}=\frac{ln{\left(\frac{r_2}{r_1}\right)}}{2\pi L k}
\end{equation}

where $r_2>r_1$ are the radial coordinates associated with the outer and inner radii
of the cylindrical annulus, $L$ is the height of the annulus, and $k$ is the
thermal conductivity of the annulus material.
