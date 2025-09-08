# DelayedGroupFilter

!alert note
`DelayedGroupFilter` can only be added to problems when the input file contains a [OpenMCCellAverageProblem](OpenMCCellAverageProblem.md)
in the `[Problem]` block. Otherwise, attempting to add this filter will result in an error.

## Description

The `DelayedGroupFilter` object provides a thin wrapper around an OpenMC [DelayedGroupFilter](https://docs.openmc.org/en/stable/pythonapi/generated/openmc.DelayedGroupFilter.html) which local tallies (added through the [tally system](AddTallyAction.md)) can access. The delayed groups tallied over can be
specified by setting `dnp_groups`. Presently, only the ENDF delayed group structure is supported; valid groups are therefore 1, 2, 3, 4, 5, and 6.

## Example Input File Syntax

As an example, a [CellTally](CellTally.md) named `DNP` applies an `DelayedGroupFilter` named `Delayed` to break up the decay rate
and delayed neutron source term into the ENDF six group structure.

!listing /tests/neutronics/filters/delayed/cell.i
  block=Problem

!syntax parameters /Problem/Filters/DelayedGroupFilter

!syntax inputs /Problem/Filters/DelayedGroupFilter
