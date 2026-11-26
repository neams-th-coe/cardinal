# SpatialLegendreFilter

## Description

The `SpatialLegendreFilter` object provides a thin wrapper around an OpenMC [SpatialLegendreFilter](https://docs.openmc.org/en/stable/pythonapi/generated/openmc.SpatialLegendreFilter.html) which local tallies (added through the [tally system](AddTallyAction.md)) can access. This filter expands the tally score in Legendre polynomials of the particle's position along one of the Cartesian axes, forming a [!ac](FET) along that axis. The axis is selected with [!param](/Problem/Filters/SpatialLegendreFilter/axis), the expansion order with [!param](/Problem/Filters/SpatialLegendreFilter/order), and the extent of the expansion with [!param](/Problem/Filters/SpatialLegendreFilter/minimum) and [!param](/Problem/Filters/SpatialLegendreFilter/maximum).

Like all other Cardinal objects, [!param](/Problem/Filters/SpatialLegendreFilter/minimum) and [!param](/Problem/Filters/SpatialLegendreFilter/maximum) are specified in the units of the `[Mesh]`; they are converted to the centimeters OpenMC expects with [!param](/Problem/scaling). OpenMC does not score any contribution from particles which are outside of these limits.

Instead of specifying the limits explicitly, setting [!param](/Problem/Filters/SpatialLegendreFilter/track_mesh) takes them from the extent of the `[Mesh]` along the chosen axis and refreshes them every time the tallies are re-initialized. This is required when the domain deforms during the solve -- for instance a thermally expanding geometry re-skinned by a [MoabSkinner](MoabSkinner.md) -- because the limits are otherwise fixed at their initial values and the expansion would stop spanning the domain as it grows. [!param](/Problem/Filters/SpatialLegendreFilter/block) restricts that extent to a set of subdomains, which is how an element-local expansion is built; see [SpatialLegendreAux](SpatialLegendreAux.md).

Unlike the spatial filters added by the tally objects themselves (such as the cell instance filter added by a [CellTally](CellTally.md)), this filter does not define an OpenMC to MOOSE mapping. Instead, it subdivides each mapped tally bin into a set of expansion coefficients, which are stored in auxvariables named with a `_xl0`, `_xl1`, ... suffix (or `yl`/`zl`, depending on the axis). Because the coefficients of the $l > 0$ moments are not averages, they are skipped when Cardinal normalizes the tally; the $l = 0$ moment is used for normalization.

The expansion coefficients can be evaluated back onto the `[Mesh]` using a [SpatialLegendreAux](SpatialLegendreAux.md).

!alert note
OpenMC scores this filter using the particle's position, which is only meaningful for collision
and analog estimators. Cardinal will error if [!param](/Problem/Tallies/CellTally/estimator) is
set to `tracklength` on a tally using a `SpatialLegendreFilter`.

## Example Input File Syntax

As an example, the `Expansion` [CellTally](CellTally.md) below applies a `SpatialLegendreFilter` to compute a Legendre expansion of the scattering reaction rate along the $z$ axis, over $-2 \le z \le 10$ cm.

!listing /tests/neutronics/filters/spatial_legendre/reconstruct_order0.i
  block=Problem

!syntax parameters /Problem/Filters/SpatialLegendreFilter

!syntax inputs /Problem/Filters/SpatialLegendreFilter
