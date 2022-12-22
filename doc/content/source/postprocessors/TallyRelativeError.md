# TallyRelativeError

!syntax description /Postprocessors/TallyRelativeError

## Description

This postprocessor evaluates the maximum or minimum relative error in the
tally created by [OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md).
This postprocessor can be used to monitor statistical convergence of the tally.

## Example Input Syntax

Below, the `max_rel_err` and `min_rel_err` postprocessors are used to
evaluate the maximum and minimum tally relative error, respectively.

!listing test/tests/postprocessors/fission_tally_relative_error/openmc.i
  block=Postprocessors

!syntax parameters /Postprocessors/TallyRelativeError

!syntax inputs /Postprocessors/TallyRelativeError

!syntax children /Postprocessors/TallyRelativeError
