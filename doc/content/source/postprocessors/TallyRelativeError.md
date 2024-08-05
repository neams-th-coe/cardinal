# TallyRelativeError

!syntax description /Postprocessors/TallyRelativeError

## Description

This postprocessor evaluates the maximum, minimum, or average relative error in the
tally created by [OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md).
This postprocessor can be used to monitor statistical convergence of the tally.

## Example Input Syntax

Below, the `max_rel_err`, `min_rel_err` and `avg_rel_err` postprocessors are used to
evaluate the maximum, minimum and average tally relative error, respectively.

!listing test/tests/postprocessors/fission_tally_relative_error/openmc.i
  block=Postprocessors

!syntax parameters /Postprocessors/TallyRelativeError

!syntax inputs /Postprocessors/TallyRelativeError

!syntax children /Postprocessors/TallyRelativeError
