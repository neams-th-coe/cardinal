# SphericalHarmonicsFilter

## Description

The `SphericalHarmonicsFilter` object provides a thin wrapper around an OpenMC [SphericalHarmonicsFilter](https://docs.openmc.org/en/stable/pythonapi/generated/openmc.SphericalHarmonicsFilter.html) which local tallies (added through the [tally system](AddTallyAction.md)) can access. You can add a spherical harmonics expansion of up to order 10, which is the maximum order supported by OpenMC.

## Example Input File Syntax

As an example, a [CellTally](CellTally.md) named `Flux` applies an `SphericalHarmonicsFilter` named `SH` to compute the scalar flux and the three
components of the net current (an expansion of order one).

!listing /tests/neutronics/filters/sh/openmc_l1.i
  block=Problem

!syntax parameters /Problem/Filters/SphericalHarmonicsFilter

!syntax inputs /Problem/Filters/SphericalHarmonicsFilter
