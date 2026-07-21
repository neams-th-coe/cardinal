# OpenMCCellMaterialFill

## Description

`OpenMCCellMaterialFill` is a [ModelModifier](AddModelModifiersAction.md) that
sets the fill of an OpenMC cell to a user-specified material or list of materials
(for distributed cells), prior to execution. If the cell in question
is a distribcell (has multiple instances throughout the geometry), then the material
in each instance can be individually modified (by providing multiple values for
`material_ids`) or the same material can be applied to all distribcells at once
(by providing a single entry in `material_ids`).

## Example Input File Syntax

The following input will modify the material in cell 2; there are three instances of cell
2, and each is modified to material 1.

!listing test/tests/model_modifiers/cell_material_fill/openmc.i
  block=Problem

## A note about modifying via file
The `material_ids_file` parameter is powerful for larger models where it would be too tedious and extremely difficult to verify using the `material_ids` param. However, this requires care when creating the HDF5 (`*.h5`) file. For TRISO problems specifically, there is a bit of nuance to be aware of. As soon as there is a reason to vary TRISO material composition throughout the reactor (e.g. to study the effects of depletion or to test loading different enrichments), it becomes necessairy to use a distribmat list to specify what `openmc.Materials` go in which TRISO instance. Models not concerned with this can use the same fuel material in every TRISO in the problem and do not need this feature.

OpenMC allows specifiying the distribmat list of a cell via `openmc.Cell.fill = [...]`.  This list should be **constructed carefully**, and once complete, it can be used to generate the HDF5 file used in Cardinal (see section below). The list needs to have as many entries as there are fuel spheres in the entire model. Say there are `N` TRISO cells per fuel cyilnder, then the list `[...]` needs to start with `N` entries corresponding to the material ("zone") in the first fuel compact. The next `N` entries should correspond to whatever material goes in the second fuel compact, and so on. **It is highly reccomended to use the OpenMC Plotter when figuring out the order cell instance increases throughout the cells in your model.** 

If the number of materials does not line up with the instances correctly, then the materials will be split between different fuel cylinders, which is not correct. A note about the instances of a TRISO cell when using `create_triso_lattice` Say there are `M` physical TRISOs that are inside the cylinder. The number of instances `N` will be greater than `M` because of the way that instances are split when a TRISO particle lives between boundaries in the search lattice. Make sure to verify the number of **instances** of your TRISO cell because this is how many entires need to exist for each fuel compact, not the actual physical number of spheres in the fuel cylinder.

## How to automate openmc.Cell.fill to HDF5
Here is an exmaple of how to take a model that uses distribmat to specify `openmc.Cell.fill` and to export that data to HDF5.

!listing /doc/content/source/modelmodifiers/openmc_to_hdf5.py language=python

!syntax parameters /Problem/ModelModifiers/OpenMCCellMaterialFill

!syntax inputs /Problem/ModelModifiers/OpenMCCellMaterialFill
