# OpenMCCellMaterialFill

## Description

`OpenMCCellMaterialFill` is a [ModelModifier](AddModelModifiersAction.md) that
changes a cell's material fill before executing Cardinal.

## Example Input File Syntax

The following input will modify the material in cell 2; there are three instances of cell
2, and each is modified to material 1.

!listing test/tests/model_modifiers/cell_material_fill/openmc.i
  block=Problem

!syntax parameters /Problem/ModelModifiers/OpenMCCellMaterialFill

!syntax inputs /Problem/ModelModifiers/OpenMCCellMaterialFill
