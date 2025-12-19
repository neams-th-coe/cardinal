# OpenMCCellTransform

!syntax description /UserObjects/OpenMCCellTransform

## Description

This user object can be used to either translate or rotate an `openmc.Universe` which is the fill of an `openmc.Cell`.

!alert! warning title=The specified transformation is only applied to the universe which fills the cell.
This userobject doesn't translate or rotate the surfaces of the specified cell. Instead, the given transform will be used to shift/rotate the universe fill. Note that it is possible to create undesired void regions with a poorly defined cell/universe/transformation combination, which may result in lost particles. If you want to use moving mesh capabilities for general cell surfaces, consider using the [MoabSkinner](MoabSkinner.md) with DAGMC geometries instead.
!alert-end!

## Example Input Syntax

To translate a universe, the transform array takes ($\Delta x$, $\Delta y$, $\Delta z$) translations. Below is an example which will translate the universe inside a specified OpenMC cell:

!listing test/tests/userobjects/openmc_cell_transform/translate_cells.i
  block=UserObjects

To rotate a universe, the transform array takes ($\phi$, $\theta$, $\psi$) which are the rotation angles in degrees about x, y, and z axes. Below is an example which will rotate the universe inside a specified OpenMC cell:

!listing test/tests/userobjects/openmc_cell_transform/rotate_cells.i
  block=UserObjects

!syntax parameters /UserObjects/OpenMCCellTransform

!syntax inputs /UserObjects/OpenMCCellTransform

!syntax children /UserObjects/OpenMCCellTransform

