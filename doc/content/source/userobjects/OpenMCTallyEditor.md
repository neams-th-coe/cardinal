# OpenMCTallyEditor

!syntax description /UserObjects/OpenMCTallyEditor

## Description

This user object can be used to change the nuclides, filters, and scores on a
given OpenMC tally. This object is primarily used for facilitating depletion
coupling of OpenMC to MOOSE, where we need to register new tallies with
controllable parameters to account for changing compositions during depletion.

## Example Input Syntax

Below is an example which will create an OpenMC tally with controllable parameters
to only obtain the fission heating from U-238 (ignoring the
portion from U-235 in this particular tally).

!listing test/tests/userobjects/openmc_tally_editor/nuclide_absorption.i
  block=UserObjects

!syntax parameters /UserObjects/OpenMCTallyEditor

!syntax inputs /UserObjects/OpenMCTallyEditor

!syntax children /UserObjects/OpenMCTallyEditor
