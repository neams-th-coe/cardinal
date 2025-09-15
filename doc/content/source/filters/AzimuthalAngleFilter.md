# AzimuthalAngleFilter

## Description

The `AzimuthalAngleFilter` object provides a thin wrapper around an OpenMC [AzimuthalFilter](https://docs.openmc.org/en/stable/pythonapi/generated/openmc.AzimuthalFilter.html#openmc.AzimuthalFilter) which local tallies (added through the [tally system](AddTallyAction.md))
can access.  Two bin options are available; equally spaced bins (setting `num_equal_divisions`) or
custom bins (setting `azimuthal_angle_boundaries`).

## Example Input File Syntax

As an example, a [MeshTally](MeshTally.md) named `Flux` applies an `AzimuthalAngleFilter` named `Azimuthal` to break the total flux up
into two hemispheres.

!listing /tests/neutronics/filters/azimuthal/mesh.i
  block=Problem

!syntax parameters /Problem/Filters/AzimuthalAngleFilter

!syntax inputs /Problem/Filters/AzimuthalAngleFilter
