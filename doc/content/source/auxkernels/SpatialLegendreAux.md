# SpatialLegendreAux

!syntax description /AuxKernels/SpatialLegendreAux

## Description

This auxkernel evaluates a tally which has been expanded in Legendre polynomials of position by a [SpatialLegendreFilter](SpatialLegendreFilter.md). The filter stores a set of expansion coefficients $a_l$ in auxvariables; this object sums the expansion back into a spatially-varying field,

\begin{equation}
\label{eq:legendre_reconstruction}
f(x) = \sum_{l=0}^{N}\frac{2l + 1}{x_{max} - x_{min}}a_{l}P_{l}\left(\hat{x}\right)
\end{equation}

where $N$ is the expansion order, $P_{l}$ are the Legendre polynomials, $x_{min}$ and $x_{max}$ are the expansion limits set on the filter, and $\hat{x}$ is the coordinate along the expansion axis normalized onto $[-1, 1]$,

\begin{equation}
\label{eq:legendre_norm_coord}
\hat{x} = \frac{2\left(x - x_{min}\right)}{x_{max} - x_{min}} - 1
\end{equation}

The weighting applied to each moment is selected with [!param](/AuxKernels/SpatialLegendreAux/normalization):

- `volumetric` (the default) multiplies moment $l$ by $2l + 1$. This is the correct setting for the tally variables Cardinal writes, which have *already* been divided by the mapped volume of their tally bin. That volume carries the length of the expansion domain, so the $1/(x_{max} - x_{min})$ factor of the Legendre series has already been applied and must not be applied a second time.
- `raw` multiplies moment $l$ by $\frac{2l + 1}{x_{max} - x_{min}}$, the full series coefficient, matching OpenMC's `openmc.legendre_from_expcoef` Python utility. Use this only for coefficients that have not been divided by a volume.
- `none` sums the polynomials against the coefficients with no additional factors.

Choosing the wrong weighting does not merely rescale the result; it changes the relative weight of each moment and therefore the reconstructed *shape*. Points outside of $[x_{min}, x_{max}]$ evaluate to zero, because OpenMC does not score any contribution there.

The score to evaluate is selected with [!param](/AuxKernels/SpatialLegendreAux/score). If multiple tallies accumulate that score, the tally must be disambiguated with [!param](/AuxKernels/SpatialLegendreAux/tally); likewise, if a tally has more than one `SpatialLegendreFilter` applied, the filter must be selected with [!param](/AuxKernels/SpatialLegendreAux/filter). If the tally has *other* filters applied in addition to the Legendre filter (such as an [EnergyFilter](EnergyFilter.md)), [!param](/AuxKernels/SpatialLegendreAux/ext_filter_bin) selects which of those bins to reconstruct; the default of 0 evaluates the first.

### Element-local expansions

A `SpatialLegendreFilter` spans a single interval, and every bin of the tally it is attached to shares that one polynomial basis. If the basis spans the whole domain while the tally is a [CellTally](CellTally.md) covering many cells, each cell's bins hold only a *partial* integral of the global basis, and evaluating the series cell-by-cell does not reconstruct the score.

To expand a score *within* each element, give each cell its own tally and its own filter restricted to that cell with [!param](/Problem/Filters/SpatialLegendreFilter/block), so that the basis spans just that cell. A `SpatialLegendreAux` restricted to the same block then reconstructs a genuine sub-cell shape, and `normalization = volumetric` conserves the integral of the score.

### Deforming domains

The expansion limits are read from the filter on every evaluation rather than being cached, so a filter using [!param](/Problem/Filters/SpatialLegendreFilter/track_mesh) on a domain which deforms during the solve continues to reconstruct correctly as the domain grows. This matters because the reconstruction evaluates to zero outside $[x_{min}, x_{max}]$: were the limits cached, every element which had expanded past the original extent would silently reconstruct as zero, losing part of the score. That loss is easy to miss, because the reconstruction remains exact everywhere *inside* the stale extent. Verify a coupled reconstruction by checking that its integral matches the expected total (for instance a prescribed power), not by inspecting the shape of the field.

### Higher-order variables

Unlike most of Cardinal's tally auxkernels, [!param](/AuxKernels/SpatialLegendreAux/variable) is not restricted to `CONSTANT MONOMIAL`. Because [!eqref](eq:legendre_reconstruction) is a continuous function of position, it is evaluated at each quadrature point, so the expansion can be projected onto an elemental variable of any order (for example `FIRST MONOMIAL` or `FIRST L2_LAGRANGE`) to recover the sub-element shape the expansion represents. This is the main reason to use a spatial expansion in the first place: it resolves variation *within* each mapped tally bin, which an unfiltered tally cannot. Nodal variables cannot be used, because the tally coefficient variables this object reads are elemental.

## Example Input Syntax

Below, a `SpatialLegendreAux` evaluates the $z$-direction Legendre expansion of the `scatter` score accumulated by the `Expansion` tally.

!listing /tests/neutronics/filters/spatial_legendre/reconstruct_order0.i
  block=AuxKernels

!syntax parameters /AuxKernels/SpatialLegendreAux

!syntax inputs /AuxKernels/SpatialLegendreAux
