# OpenMCCellTransform

!syntax description /UserObjects/OpenMCCellTransform

## Description

This user object can be used to either translate or rotate a universe inside an OpenMC cell.

!alert! warning title=The specified cell must be filled with a universe which is what will be transformed
This userobject doesn't translate or rotate the surfaces of the specifeid cell. Instead, if that cell is filled with a universe, the given transform will be used to shift/rotate that universe. If you want to use moving mesh capabilities, you may consider using the MOAB skinner with DAGMC geometries instead.
!alert-end!

## Example Input Syntax

To translate a universe, the transform array takes ($\Delta x$, $\Delta y$, $\Delta z$) translations. Below is an example which will translate the universe inside a specified OpenMC cell:

!listing test/tests/userobjects/openmc_cell_transform/translate_cells.i
  block=UserObjects

To rotate a universe, the transform array takes ($\phi$, $\theta$, $\psi$) which are the rotation angles in degrees about x, y, and z axes. Below is an example which will translate the universe inside a specified OpenMC cell:

!listing test/tests/userobjects/openmc_cell_transform/rotate_cells.i
  block=UserObjects

!syntax parameters /UserObjects/OpenMCCellTransform

!syntax inputs /UserObjects/OpenMCCellTransform

!syntax children /UserObjects/OpenMCCellTransform

