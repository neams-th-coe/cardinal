# EnergyFilter

!alert note
`EnergyFilter` can only be added to problems when the input files contains a [OpenMCCellAverageProblem](OpenMCCellAverageProblem.md)
in the `[Problem]` block. Otherwise, attempting to add this filter will result in an error.

## Description

The `EnergyFilter` object provides a thin wrapper around an OpenMC `EnergyFilter` which local tallies (added through the
[tally system](AddTallyAction.md)) can access. This filter bins particles based off of the particle energy before the collision
has occured (for collision or analog estimators) or the particle energy (for tracklength estimators). The ordering of the bins in
`EnergyFilter` can be reversed (set to be descending in terms of energy) by setting `reverse_bins = true` if you wish to use
the conventional multi-group radiation transport formalism.

## Example Input File Syntax

As an example, a [MeshTally](MeshTally.md) named `Flux` applies an `EnergyFilter` named `Energy` to break the total flux up
into multi-group fluxes using the CASMO-2 group structure.

!listing /tests/neutronics/filters/energy/mesh.i
  block=Problem

!syntax parameters /Problem/Filters/EnergyFilter

!syntax inputs /Problem/Filters/EnergyFilter

!syntax children /Problem/Filters/EnergyFilter
