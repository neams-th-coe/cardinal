# FissionTallyRelativeError

!syntax description /Postprocessors/FissionTallyRelativeError

## Description

This postprocessor evaluates the maximum or minimum relative error in the fission
tally created by [OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md).
This postprocessor can be used to monitor statistical convergence of the fission
tally used for multiphysics coupling to MOOSE.

## Example Input Syntax

Below, the `max_rel_err` and `min_rel_err` postprocessors are used to
evaluate the maximum and minimum fission tally relative error, respectively.

!listing test/tests/postprocessors/fission_tally_relative_error/openmc.i
  block=Postprocessors

!syntax parameters /Postprocessors/FissionTallyRelativeError

!syntax inputs /Postprocessors/FissionTallyRelativeError

!syntax children /Postprocessors/FissionTallyRelativeError
