# AddFilterAction

!alert note
`AddFilterAction` can only add filters to problems which contain a [OpenMCCellAverageProblem](OpenMCCellAverageProblem.md)
in the `[Problem]` block. Otherwise, attempting to add a filter will result in an error.

## Overview

The `AddFilterAction` action is responsible for adding filters which can be accessed by local tallies. This is done with the
`[Filters]` block in a Cardinal input file.

## Example Input File Syntax

As an example, an [EnergyFilter](EnergyFilter.md) and [PolarAngleFilter](PolarAngleFilter.md) are added in the
`[Problem/Filters]` block which are then used by a [MeshTally](MeshTally.md).

!listing /tests/neutronics/filters/multi_filter.i
  block=Problem

!syntax list /Problem/Filters actions=false subsystems=false heading=Available Filter Objects

!syntax parameters /Problem/Filters/AddFilterAction
