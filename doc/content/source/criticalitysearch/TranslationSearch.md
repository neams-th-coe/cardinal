# TranslationSearch

## Description

`TranslationSearch` is a [CriticalitySearch](AddCriticalitySearchAction.md) that targets a specified $k$-eigenvalue by updating an `openmc.Cell.translation`, which can be applied to one or more cells in the OpenMC model. The `TranslationSearch` class inherits from `OpenMCCellTransformBase`, which handles all modifications to the cells involved in a search.

!alert note
The cells specified in the search must be filled with a universe. All cells included in the search will have the same translation applied.

Only axis aligned searches are allowed, so the user needs to specify which axis (`x`, `y`, or `z`) the fill will be translated along via the [!param](/Problem/CriticalitySearch/TranslationSearch/translation_axis) parameter when searching for a critical configuration. The [!param](/Problem/CriticalitySearch/TranslationSearch/minimum), [!param](/Problem/CriticalitySearch/TranslationSearch/maximum), and [!param](/Problem/CriticalitySearch/TranslationSearch/root_tol) parameters, as well as the `critical_value` postprocessor, are all in the length units of the `[Mesh]`. They are converted to OpenMC's units of centimeters using the `scaling` parameter of [OpenMCCellAverageProblem](OpenMCCellAverageProblem.md).

!alert! warning title=The translation replaces any translation already set on the cell.
The value being searched over is the cell's absolute translation along the search axis, not a displacement relative to the translation specified in the OpenMC model. The translation components along the other two axes are set to zero. If the fill of your cell is already offset in the OpenMC model, build the model such that this offset is instead part of the definition of the universe.
!alert-end!

!alert! warning title=The translation is only applied to the universe which fills the cell.
The surfaces which bound the specified cell(s) do not move. Every surface in the universe filling the cell is translated, so it is possible to create undesired void regions which may result in lost particles. For example, if the cells in the universe are bounded by the same surfaces as the cell they fill, any non-zero translation will uncover a void. The universe should instead extend past the bounds of the cell it fills by at least the range being searched.
!alert-end!

## Example Input File Syntax

Here is a valid `TranslationSearch` which shows the corresponding `CriticalitySearch` block used to define the search which will happen on each iteration.

!listing test/tests/criticality/translation/openmc.i
  block=Problem

!syntax parameters /Problem/CriticalitySearch/TranslationSearch

!syntax inputs /Problem/CriticalitySearch/TranslationSearch
