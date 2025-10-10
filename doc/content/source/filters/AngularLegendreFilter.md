# AngularLegendreFilter

## Description

The `AngularLegendreFilter` object provides a thin wrapper around an OpenMC [LegendreFilter](https://docs.openmc.org/en/stable/pythonapi/generated/openmc.LegendreFilter.html) which local tallies (added through the [tally system](AddTallyAction.md)) can access. This filter expands the change in particle direction in Legendre polynomials of an arbitrary order.

## Example Input File Syntax

As an example, a [CellTally](CellTally.md) named `Scattering` applies an `AngularLegendreFilter` named `Legendre` to compute a Legendre expansion of
the scattering reaction rate with order 1.

!listing /tests/neutronics/filters/legendre/cell.i
  block=Problem

!syntax parameters /Problem/Filters/AngularLegendreFilter

!syntax inputs /Problem/Filters/AngularLegendreFilter
