The NekRS solution in element $e$, or $f_e$, is a linear combination of polynomials $\phi$ and coefficients $c$,

\begin{equation}
\label{eq:poly}
f_e(x)=\sum_{i\ =\ 0}^Nc_i\phi_i(x)
\end{equation}

where $N$ is the polynomial order. In NekRS, the polynomials are Lagrange interpolants of [!ac](GLL)
quadrature points, $x_i$ for $0\leq i\leq N$ such that $\phi_i(x_j)=\delta_{ij}$. To help explain
the interpolation between NekRS's high-order solution and the lower-order mesh mirror,
[interpolation] shows a 1-D element with a third-order NekRS solution (the black curved line) based
on four [!ac](GLL) points (blue dots).

!media interpolation.png
  id=interpolation
  caption=Interpolation between a high-order NekRS solution (based on [!ac](GLL) quadrature points $x$) and generic points $p$
  style=width:40%;margin-left:auto;margin-right:auto;halign:center

To interpolate this solution to a MOOSE element with arbitrary
nodes $p$ (red dots),

\begin{equation}
\label{eq:vm}
\begin{bmatrix}
f(p_0)\\
f(p_1)
\end{bmatrix}
=\begin{bmatrix}
\phi_0(p_0)& \phi_1(p_0) & \phi_2(p_0) & \phi_3(p_0)\\
\phi_0(p_1)& \phi_1(p_1) & \phi_2(p_1) & \phi_3(p_1)
\end{bmatrix}
\begin{bmatrix}
c_0\\
c_1\\
c_2\\
c_3
\end{bmatrix}\ .
\end{equation}

The first matrix on the right-hand side of [eq:vm] is referred to as a [Vandermonde matrix](https://en.wikipedia.org/wiki/Vandermonde_matrix).
The interpolation is performed by evaluating the NekRS solution using the polynomial solution in [eq:poly]
 at the MOOSE nodes using Vandermonde matrices.

This formulation is then extended to the problem-specific polynomial order, where the NekRS
quadrature points are determined based on the poynomial order (since NekRS always uses the
[!ac](GLL) quadrature) and the MOOSE quadrature points are determined based on the combination
of the order of the variables `temp`, `avg_flux`, and `heat_source` plus the quadrature rule
specified in the input file.
