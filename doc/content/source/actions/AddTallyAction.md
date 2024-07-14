# AddTallyAction

!alert note
`AddTallyAction` can only add tallies to problems which contain a [OpenMCCellAverageProblem](OpenMCCellAverageProblem.md)
in the `[Problem]` block. Otherwise, attempting to add a tally will result in an error.

## Overview

The `AddTallyAction` action is responsible for adding local tallies to a simulation through the
`[Tallies]` block in a Cardinal input file.

For all requested tallies other than unstructured mesh tallies, a single tally object is added
per sub-block in `[Tallies]`. When an unstructured mesh tally is requested `N` tally objects
will be added, where `N` is the number of positions in `mesh_translations` / the number of positions
in the file named `mesh_translations_file`. These translated mesh tallies store their scores in the
same auxvariable(s), and the tally scores are normalized by the sum over all bins in the associated
[OpenMCCellAverageProblem](OpenMCCellAverageProblem.md). `mesh_translations` and
`mesh_translations_file` are only applicable to [MeshTally](MeshTally.md) objects.

## Example Input File Syntax

An example where multiple unstructured mesh tallies are added can be found below. `AddTallyAction`adds
three mesh tallies, where each tally has one of the positions listed in the file named `mesh_translations_file`.

!listing /tutorials/pebbles/openmc_um.i
  block=Tallies

An example of adding non-translated tallies can be found below, where both a [CellTally](CellTally.md)
and [MeshTally](MeshTally.md) are added to the problem.

!listing /multi_tally_example.i
  block=Tallies

!syntax list /Tallies actions=false subsystems=false heading=Available Tally Objects

!syntax parameters /Tallies/AddTallyAction
