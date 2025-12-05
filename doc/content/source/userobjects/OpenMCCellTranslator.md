# OpenMCCellTranslator

!syntax description /UserObjects/OpenMCCellTranslator

## Description

This user object can be used to translate a universe inside an OpenMC cell.

!alert! warning title=The specified cell must be filled with a universe which is what will be translated
This userobject doesn't translate the specifeid cell surfaces. Instead, if that cell is filled with a universe, the given translations will be used to shift that universe. If you want to use moving mesh capabilities, you may consider using the MOAB skinner with DAGMC geometries instead.
!alert-end!

## Example Input Syntax

Below is an example which will translate the universe inside a specified OpenMC cell

!listing test/tests/userobjects/openmc_cell_translator/translate_cells.i
  block=UserObjects

!syntax parameters /UserObjects/OpenMCCellTranslator

!syntax inputs /UserObjects/OpenMCCellTranslator

!syntax children /UserObjects/OpenMCCellTranslator

