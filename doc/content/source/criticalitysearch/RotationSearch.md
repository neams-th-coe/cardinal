# RotationSearch

## Description

`RotationSearch` is a [CriticalitySearch](AddCriticalitySearchAction.md) that targets a specified $k$-eigenvalue by updating an `openmc.Cell.rotation`, which can be applied to one or more cells in the OpenMC model. The `RotationSearch` class inherits from `OpenMCCellTransformBase`, which handles all modifications to the cells involved in a search.

!alert note
The cells specified in the search must be filled with a universe. All cells included in the search will have the same rotation applied.

Only axis aligned searches are allowed, so the user needs to specify which rotation axis ( `x`, `y`, or `z`) will be modified when searching for a critical configuration. The main reason for this restriction is that it is very easy to create an OpenMC model with voids prone to lost particles by modifying multiple rotation angles at once. If your model desires to rotate around a non-principal axis or rotate around multiple axes, it is advised to instead build a version in a different reference frame that searches by rotating along one of the three available axes.

## Example Input File Syntax

Here is a valid `RotationSearch` which shows the corresponding `CriticalitySearch` block used to defines the search which will happen on each iteration.

!listing test/tests/criticality/rotation/openmc.i
  block=Problem

!syntax parameters /Problem/CriticalitySearch/RotationSearch

!syntax inputs /Problem/CriticalitySearch/RotationSearch
