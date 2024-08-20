# PolarAngleFilter

!alert note
`PolarAngleFilter` can only be added to problems when the input file contains a [OpenMCCellAverageProblem](OpenMCCellAverageProblem.md)
in the `[Problem]` block. Otherwise, attempting to add this filter will result in an error.

## Description

The `PolarAngleFilter` object provides a thin wrapper around an OpenMC [PolarFilter](https://docs.openmc.org/en/stable/pythonapi/generated/openmc.PolarFilter.html) which local tallies (added through the [tally system](AddTallyAction.md)) can access. Two
bin options are available; equally spaced bins (setting `num_equal_divisions`) or custom bins
(setting `polar_angle_boundaries`).

## Example Input File Syntax

As an example, a [MeshTally](MeshTally.md) named `Flux` applies an `PolarAngleFilter` named `Polar` to break the total flux up
into two hemispheres.

!listing /tests/neutronics/filters/polar/mesh.i
  block=Problem

!syntax parameters /Problem/Filters/PolarAngleFilter

!syntax inputs /Problem/Filters/PolarAngleFilter

!syntax children /Problem/Filters/PolarAngleFilter
