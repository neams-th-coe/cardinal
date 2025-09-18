# EnergyOutFilter

## Description

The `EnergyOutFilter` object provides a thin wrapper around an OpenMC [EnergyoutFilter](https://docs.openmc.org/en/stable/pythonapi/generated/openmc.EnergyoutFilter.html) which local tallies (added through the [tally system](AddTallyAction.md)) can access. Bins can be provided
by setting `energy_boundaries` or by selecting an energy group structure in `group_structure`; more information
on these group structures can be found in the [OpenMC multi-group cross section documentation](https://docs.openmc.org/en/stable/pythonapi/mgxs.html).
The ordering of the bins in `EnergyOutFilter` can be reversed (set to be  descending in terms of energy)
by setting `reverse_bins = true` if you wish to use the conventional multi-group radiation transport formalism.

## Example Input File Syntax

As an example, a [CellTally](CellTally.md) named `Scattering` applies an `EnergyOutFilter` named `EnergyOut` to bin the outgoing energies from
scattered particles into two groups using the CASMO-2 group structure.

!listing /tests/neutronics/filters/energy_out/cell.i
  block=Problem

!syntax parameters /Problem/Filters/EnergyOutFilter

!syntax inputs /Problem/Filters/EnergyOutFilter
