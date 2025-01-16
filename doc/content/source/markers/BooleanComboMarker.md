# BooleanComboMarker
  id=bcm

## Description

The `BooleanComboMarker` class combines multiple other markers together using boolean operations.
The markers provided in `refine_markers` are responsible for determining if an element should be refined, while the
markers provided in `coarsen_markers` are responsible for determining if an element should be coarsened. Setting
`boolean_operator` to `and` will require all `refine_markers` to return `MarkerValue::REFINE` before
`BooleanComboMarker` marks an element for refinement. Setting`boolean_operator` to `or` will require a single marker
to return `MarkerValue::REFINE` before an element is marked for refinement. The same holds true for `coarsen_markers`
with `MarkerValue::COARSEN`. In the event both the refinement and coarsening expressions are true, the preferred refinement
operation (`priority`) is used to determine if the element is marked for refinement or coarsening.

## Example Input File Syntax

!listing /tests/markers/boolean_combo/prioritize_refine_and.i
  block=Markers

!syntax parameters /Adaptivity/Markers/BooleanComboMarker

!syntax inputs /Adaptivity/Markers/BooleanComboMarker
