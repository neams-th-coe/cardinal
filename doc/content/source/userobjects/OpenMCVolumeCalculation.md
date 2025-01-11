# OpenMCVolumeCalculation

!syntax description /UserObjects/OpenMCVolumeCalculation

## Description

This user object can be combined with [OpenMCCellAverageProblem](OpenMCCellAverageProblem.md)
to compute the volumes of all OpenMC cells which map to the `[Mesh]` using a
[stochastic volume calculation](https://docs.openmc.org/en/stable/usersguide/volume.html).

!alert warning
OpenMC does not yet have the ability to individually estimate volumes of cell _instances_.
OpenMC can only stochastically estimates volumes for all instances of a given cell ID at once.
So, if your problem has repeated cells, we approximate the volume of each cell instance as
the volume across _all_ instances, divided by the number of instances. For most geometries,
this is correct - but if your geometry has any cell instances which don't "make it fully
into the geometry" (e.g. if they are clipped in some way), this calculation will yield
inaccurate volumes.

## Example Input Syntax

Below is an example which will calculate the volumes of all OpenMC cells which map to
MOOSE.

!listing test/tests/userobjects/volume_calculation/openmc.i
  start=Problem
  end=Executioner

!syntax parameters /UserObjects/OpenMCVolumeCalculation

!syntax inputs /UserObjects/OpenMCVolumeCalculation
