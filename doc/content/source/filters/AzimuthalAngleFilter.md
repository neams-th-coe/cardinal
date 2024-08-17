# AzimuthalAngleFilter

!alert note
`AzimuthalAngleFilter` can only be added to problems when the input files contains a [OpenMCCellAverageProblem](OpenMCCellAverageProblem.md)
in the `[Problem]` block. Otherwise, attempting to add this filter will result in an error.

## Description

The `AzimuthalAngleFilter` object provides a thin wrapper around an OpenMC `AzimuthalFilter` which local tallies (added through the
[tally system](AddTallyAction.md)) can access. This filter bins particles based off of the particle's azimuthal angle before the collision
has occured (for collision or analog estimators) or the particle's azimuthal angle after the collision (for tracklength estimators). OpenMC
uses a coordinate system where the polar angle is defined based off of the z-axis, and the azimuthal angle is defined based on the x-y plane.
Two bin options are available; equally spaced bins (setting `num_equal_divisions`) or custom bins (setting `azimuthal_angle_boundaries`).

## Example Input File Syntax

As an example, a [MeshTally](MeshTally.md) named `Flux` applies an `AzimuthalAngleFilter` named `Azimuthal` to break the total flux up
into two hemispheres.

!listing /tests/neutronics/filters/azimuthal/mesh.i
  block=Problem

!syntax parameters /Problem/Filters/AzimuthalAngleFilter

!syntax inputs /Problem/Filters/AzimuthalAngleFilter

!syntax children /Problem/Filters/AzimuthalAngleFilter