# BoratedWater

## Description

`BoratedWater` is a [CriticalitySearch](AddCriticalitySearchAction.md) that
performs a criticality search based on the natural boron weight ppm in water.
This object will clear the nuclides in the provided material, and overwrite
with natural water and natural boron, to the specific boron weight ppm.
The density of the material remains unchanged (but it may still be being set by a
coupled thermal-fluid application).

!alert note
This object does not search in your cross section library to see what nuclides
are present (some libraries do not include O18, for instance). OpenMC will
throw an error if your library is missing a nuclide that `BoratedWater` is
trying to add. To circumvent this, please explicitly note which of the natural
nuclides from hydrogen (H1, H2), boron (B10, B11), and oxygen (O16, O17, O18)
are missing from your material with `absent_nuclides`. Currently, only O18 is supported.

!alert warning
This object does not currently add S(a,b) tables. To correctly account for thermal scattering in water, the starting material definition in OpenMC must include the correct thermal scattering library.

## Example Input File Syntax

The following input will perform a criticality search for each OpenMC solve
by searching via the boron ppm of material 2.

!listing test/tests/criticality/borated_water/openmc.i
  block=Problem

!syntax parameters /Problem/CriticalitySearch/BoratedWater

!syntax inputs /Problem/CriticalitySearch/BoratedWater
