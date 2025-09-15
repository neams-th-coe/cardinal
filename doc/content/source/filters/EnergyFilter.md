# EnergyFilter

## Description

The `EnergyFilter` object provides a thin wrapper around an OpenMC [EnergyFilter](https://docs.openmc.org/en/stable/pythonapi/generated/openmc.EnergyFilter.html) which local tallies (added through the [tally system](AddTallyAction.md)) can access. Bins can be provided
by setting `energy_boundaries` or by selecting an energy group structure in `group_structure`; more information
on these group structures can be found in the [OpenMC multi-group cross section documentation](https://docs.openmc.org/en/stable/pythonapi/mgxs.html).
The ordering of the bins in `EnergyFilter` can be reversed (set to be  descending in terms of energy)
by setting `reverse_bins = true` if you wish to use the conventional multi-group radiation transport formalism.

## Example Input File Syntax

As an example, a [MeshTally](MeshTally.md) named `Flux` applies an `EnergyFilter` named `Energy` to break the total flux up
into multi-group fluxes using the CASMO-2 group structure.

!listing /tests/neutronics/filters/energy/mesh.i
  block=Problem

!syntax parameters /Problem/Filters/EnergyFilter

!syntax inputs /Problem/Filters/EnergyFilter
