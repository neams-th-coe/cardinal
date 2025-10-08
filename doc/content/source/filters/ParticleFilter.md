# ParticleFilter

## Description

The `ParticleFilter` object provides a thin wrapper around an OpenMC [ParticleFilter](https://docs.openmc.org/en/stable/pythonapi/generated/openmc.ParticleFilter.html) which local tallies (added through the [tally system](AddTallyAction.md)) can access. This filter allows for events to
be filtered by the incident particle (provided in `particles`). This can be any combination of the following: `neutron`,
`photon`, `electron`, or `positron`.

## Example Input File Syntax

As an example, a [CellTally](CellTally.md) named `Heating` applies an `ParticleFilter` named `Particle` to bin heating
by the particle type.

!listing /tests/neutronics/filters/particle/cell.i
  block=Problem

!syntax parameters /Problem/Filters/ParticleFilter

!syntax inputs /Problem/Filters/ParticleFilter
