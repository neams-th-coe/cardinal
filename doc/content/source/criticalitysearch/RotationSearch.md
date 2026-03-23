# RotationSearch

## Description

`RotationSearch` is a [CriticalitySearch](AddCriticalitySearchAction.md) tagerts a specified $k$-eigenvalue by updating an `openmc.Cell.rotation`, which can be applied to one or more cells in the OpenMC model. 

!alert note
The cells specified in the search must be filled with a universe. All cells included in the search will have the same rotation applied.

The actual rotation is handled internally using an [OpenMCCellTransform](OpenMCCellTransform.md) UserObject, so the user does not need to (and should not) define one related to the search.

Only axis aligned searches are allowed, so the user needs to specify which rotation axis ( `x`, `y`, or `z`) will be modified when searching for a critical configuration. The main reason for this restriction is that it is very easy to create an OpenMC model with voids prone to lost particles by modifying multiple rotation angles at once. 

!alert tip
If you desire to specify rotation about more than one axis to the cell(s) involved with the search, it's reccomended to instead build your model in such a way the the rotation that changes reactivity in the search is only around one principal axis.

## Example Input File Syntax

Here is a valid `RotationSearch` which shows the corresponding `CriticalitySearch` block used to defines the search which will happen on each iteration.

!listing test/tests/criticality/rotation/openmc.i
  start=Problem
  end=UserObjects

!syntax parameters /Problem/CriticalitySearch/RotationSearch

!syntax inputs /Problem/CriticalitySearch/RotationSearch
