# OpenMCTallyNuclides

!syntax description /UserObjects/OpenMCTallyNuclides

## Description

This user object can be used to change the nuclides on a given OpenMC tally.
This object is primarily used for facilitating depletion coupling of OpenMC
to MOOSE, where we need to register new nuclides on a tally to account for
changing compositions during depletion.

## Example Input Syntax

Below is an example which will change the nuclides of the OpenMC
tally to only obtain the fission heating from U-238 (ignoring the
portion from U-235 in this particular tally).

!listing test/tests/userobjects/openmc_tally_nuclides/openmc.i
  block=UserObjects

!syntax parameters /UserObjects/OpenMCTallyNuclides

!syntax inputs /UserObjects/OpenMCTallyNuclides

!syntax children /UserObjects/OpenMCTallyNuclides
