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

## Providing materials via file

The `material_ids_file` parameter is powerful for larger models where it would be too tedious and extremely difficult to use the `material_ids` param in the input file. However, using a file-based set of materials requires care when creating the HDF5 (`*.h5`) file. For TRISO problems specifically, there is a bit of nuance to be aware of. As soon as there is a reason to vary TRISO material composition throughout the reactor (e.g. to study the effects of depletion or to test loading different enrichments), it becomes necessary to use a distribmat list to specify what OpenMC material goes in which TRISO instance. Models not concerned with this can use the same fuel material in every TRISO in the problem, and do not need this feature.

OpenMC allows specifiying the distribmat list of a cell in the python model building script via `openmc.Cell.fill = [...]`. The actual list can be used to generate an HDF5 file used by Cardinal. See this [section](#How to automate material fill to HDF5) for more details.  The list needs to have as many entries as there are cell instances in the entire model. Say there are `N` TRISO cells per fuel cyilnder, then the list `[...]` needs to start with `N` entries corresponding to the material ("zone") in the first fuel compact. The next `N` entries should correspond to whatever material goes in the second fuel compact, and so on. **It is highly recommended to use the** [OpenMC plotter](https://github.com/openmc-dev/plotter) **when figuring out the order that the cell instances increase throughout the cells in your model.**

!alert warning
The material instance list should be constructed carefully! Generally, you want the all TRISO particles within the same fuel compact to have the same material. If the ordering of materials is off, the materials may not line up as you expect. Especially in TRISO model, if the wrong number of materials are specified, the materials will not group nicely by cylinder.

!alert note
Regarding the number of instances for a TRISO cell when using OpenMC's `create_triso_lattice`: Say there are `M` physical TRISOs that are inside the cylinder. The number of instances `N` typically will be greater than `M` due to the search lattice. When a TRISO particle lives splits multiple voxels in the search lattice, there is an instance for each voxel that it is a part of to help accelerate transport. **Make sure to verify the number of instances of your TRISO fuel cell**. This is how many entries need to exist for each fuel compact, which is not the same as the actual physical number of spheres in the fuel cylinder.

## How to automate material fill to HDF5

Here is an exmaple of how to generate an HDF5 file that can be used via the `material_ids_file` parameter. In this example, there is a command line argument to specify a list of material IDs. However, in a model with a large list of materials to specify, it might be infeasible to type out all of the material IDs. In this case, if the user has a variable corresponding to the list of OpenMC materials, e.g. `mats_list`, they can generate the input parameter `mat_ids` for the `write_ids` function in `create_material_fill_ids_h5.py` by using a list comprehension `mat_ids = [mat.id for mat in mats_list]`.

!listing /test/tests/model_modifiers/cell_material_fill/create_material_fill_ids_h5.py language=python

!syntax parameters /Problem/ModelModifiers/OpenMCCellMaterialFill

!syntax inputs /Problem/ModelModifiers/OpenMCCellMaterialFill
