# Large Eddy Simulation

[!ac](LES) in NekRS uses a filter to drain energy from the lowest resolved
wavelengths. This filter essentially acts as a sub-grid scale dissipation model.
NekRS contains two options for filters, both of which exhibit spectral convergence.
Both options use the same underlying convolution operator, but apply it in different ways.
This documentation page is an abridged version of the [Nek5000 filtering documentation](https://nek5000.github.io/NekDoc/problem_setup/filter.html).

The solution in NekRS is represented on each element using a polynomial basis (Lagrange polynomials,
$\phi$). For example, in a 1-D element, the solution $u$ is

\begin{equation}
u=\sum_{i=0}^NC_i\phi_i
\end{equation}

where $C$ are coefficients. A filtered version of $u$, or $\tilde{u}$, can then be represented as

\begin{equation}
\tilde{u}=\sum_{i=0}^N\sigma_iC_iP_i
\end{equation}

where $\sigma$ are weighting factors and $P$ are the Legendre polynomials. In other
words, the nodal solution (Lagrange basis) is first interpolated to the modal Legendre basis,
the filtering is applied there, and then interpolated back.

Typically, $\sigma$ is unity for certain values of $N$ so as
to only apply the filter to the highest-frequency modes ($N>N'$),

\begin{equation}
\begin{cases}
\sigma_i=1 & i \leq N'\\
\sigma_i<1 & i > N'
\end{cases}
\end{equation}

We denote the convolution operator which represents applying the 1-D filtering operation to all
three dimensions as $G$, such that

\begin{equation}
\tilde{u}=Gu
\end{equation}

Note that because the $\sigma$ depend on the local element size, the filtering operation
is not uniform across the entire domain. Generally, you should use as minimal a filter
as possible while maintaining stability (though due to the spectral convergence, if you
use a stronger setting you will still converge in the limit of increasing polynomial order).

## Explicit Filter

NekRS's explicit filter applies the low-pass filtering operation directly to all the solution
variables on the end of each time step. This option is enabled by setting `filtering = explicit`
in the `[GENERAL]` block.
To specify this filter, the weight applied to the highest
mode is defined in terms of a `filterWeight`,

\begin{equation}
\sigma_N=1-\texttt{filterWeight}
\end{equation}

The number of modes to filter is then indicated with the `filterModes` key. NekRS will parabolically
decrease the effect of the filter for each mode lower than $N$ until $\sigma_{N'}=1$. General
recommended settings for $N\ge 5$ are `filterModes = 2` and `filterWeight = 0.05`.

## High Pass Filter

NekRS's high pass filter also uses the convolution operator $G$ to obtain a low-pass filtered signal.
The high-pass filter term is then constructed from the difference in the original signal ($u$)
and the low-pass filtered signal ($Gu$), and then subtracted from the momentum, energy, and scalar
transport equations. For example, the non-dimensional momentum equation would become

\begin{equation}
\rho^\dagger\left(\frac{\partial u_i^\dagger}{\partial t^\dagger}+u_j^\dagger\frac{\partial u_i^\dagger}{\partial x_j^\dagger}\right)=-\frac{\partial P^\dagger}{\partial x_i^\dagger}+\frac{1}{Re}\frac{\partial \tau_{ij}^\dagger}{\partial x_j^\dagger}+\rho^\dagger f_i^\dagger-\underbrace{\chi\left(u-Gu\right)}_{\text{low-pass filtered signal}}
\end{equation}

The high pass filter is enabled by setting `filtering = hpfrt` in the `[GENERAL]` block.
Here, once again you need to specify `filterModes`. However, different from the explicit filter,
the weights are strong for the high pass filter such that $\sigma_N=0$. In other words,
the meaning of the `filterWeight` key is now different, and instead is used to specify $\chi$.
Typical values used are $5\leq \chi\leq10$.

General recommended settings are `filterModes = 2` and `filterWeight = 5.0`.
That is, the high pass filter uses a strong low-pass filtering operation.
