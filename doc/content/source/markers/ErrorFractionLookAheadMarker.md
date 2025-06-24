# ErrorFractionLookAheadMarker

## Description

`ErrorFractionLookAheadMarker` extends the [ErrorFractionMarker](ErrorFractionMarker.md) from MOOSE
to include the statistical relative error from fields in stochastic solvers (i.e. OpenMC). If the
spatial error from `indicator` is within the error fraction provided in `refine` and the
post-refinement statistical relative error is below the threshold provided in `rel_error_refine`, the
element is marked for refinement.  If the spatial error is within the
error fraction `coarsen` or the statistical relative error is above
the threshold provided in `rel_error_refine`, the element is marked for coarsening. The post-refinement
error is estimated as: $e_{i+1} = \sqrt{N_{c}}e_{i}$; where $e_{i+1}$ is the post-refinement statistical
relative error, $N_{c}$ is the number of child elements generated when refining, and $e_{i}$ is the current
statistical relative error. The statistical error is taken from `stat_error_indicator`.

## Example Input File Syntax

!listing /tests/neutronics/markers/look_ahead/openmc.i
  block=Markers

!syntax parameters /Adaptivity/Markers/ErrorFractionLookAheadMarker

!syntax inputs /Adaptivity/Markers/ErrorFractionLookAheadMarker
