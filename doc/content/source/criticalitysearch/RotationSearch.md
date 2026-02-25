# OpenMCMaterialDensity

## Description

`RotationSearch` is a [CriticalitySearch](AddCriticalitySearchAction.md) that searches for a critical configuration by updating the value of one or more `openmc.Cell.rotation`(s). This is done by relying on an existing [OpenMCCellTransform](OpenMCCellTransform.md) UserObject that can update the OpenMC model with some added restrictions. If the [OpenMCCellTransform](OpenMCCellTransform.md) modifies multiple cells, the same rotation angle will be applied to each cell during the search.

Only axis aligned searches are allowed, i.e. you can only rotate cells around the `x`, `y`, or `z` axis. The main reason for this restriction is that it is very easy to create an OpenMC model with voids prone to lost particles by modifying multiple rotation angles at once. If you desire to apply more than one rotation to a cell, it's reccomended to instead build your model in such a way the the rotation that changes reactivity in the search is only around one principal axis.

While a generic `OpenMCCellTransform` allows modifications to all components of an `openmc.Cell.rotation` tuple via the `vector_value` parameter, for performing a `RotationSearch`, the `vector_value` must contain exactly two `0.0` values and one name of a `Postprocessor`, typically a `Receiver` `Postrprocesor`.

## Example Input File Syntax
Here is a valid `RotationSearch` which shows the corresponding `OpenMCCellTransform` that will be used in the search for a critical state.

!listing test/tests/criticality/rotation/openmc.i
  start=Problem
  end=UserObjects

!syntax parameters /Problem/CriticalitySearch/RotationSearch

!syntax inputs /Problem/CriticalitySearch/RotationSearch
