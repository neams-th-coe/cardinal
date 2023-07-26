# OpenMCNuclideDensities

!syntax description /UserObjects/OpenMCNuclideDensities

## Description

This user object can be used to change the nuclide composition of an OpenMC
material. Applications include any use case where a coupled MOOSE application
changes the nuclides present in a material, such as for depletion,
material movement, and soluble absorbers. If you are adding a new nuclide
that did not already exist in your problem, OpenMC will load the appropriate
cross section library on-the-fly.

!alert note
This object is only used to change the relative densities of nuclides in a
material. These nuclide compositions are changed *before* updating density
from MOOSE. So, if this material is coupled via density feedback to MOOSE,
the total density of the material will be set to whatever is specified in
the `density` auxiliary variable, immediately after you change the composition.

## Example Input Syntax

Below is an example which will change the composition of the OpenMC
material with ID 1, by adding U-238 (the original material in the
`materials.xml` file only consists of U-235).

!listing test/tests/userobjects/openmc_nuclide_densities/openmc.i
  block=UserObjects

!syntax parameters /UserObjects/OpenMCNuclideDensities

!syntax inputs /UserObjects/OpenMCNuclideDensities

!syntax children /UserObjects/OpenMCNuclideDensities
