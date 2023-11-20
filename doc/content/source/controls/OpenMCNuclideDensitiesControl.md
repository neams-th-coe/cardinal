# OpenMCNuclideDensitiesControl

!syntax description /Controls/OpenMCNuclideDensitiesControl

## Description

Changes the nuclide compositions of an [OpenMCNuclideDensities](https://cardinal.cels.anl.gov/source/userobjects/OpenMCNuclideDensities.html) user object
via a controls. This can be used to change OpenMC material compositions
as a function of time, such as for control rod movement or tritium migration.

## Example Input Syntax

As an example, the `c` Controls object will modify the 

!listing test/tests/controls/openmc_nuclide_densities/openmc.i
  start=UserObjects
  end=Postprocessors

!syntax parameters /Controls/OpenMCNuclideDensitiesControl

!syntax inputs /Controls/OpenMCNuclideDensitiesControl

!syntax children /Controls/OpenMCNuclideDensitiesControl
