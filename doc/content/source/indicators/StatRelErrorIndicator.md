# StatRelErrorIndicator

## Description

The `StatRelErrorIndicator` provides the relative error associated with the tally score to the indicator
system for use in adaptivity. A tally `score` must be specified, and if any external filter bins have been
added through the use of the [filter system](AddFilterAction.md) `ext_filter_bin` index should be provided.
Additionally, the tally scoring `score` must be outputting the relative error of the tally. This can be done
by specifying `output = unrelaxed_tally_rel_error` in the tally object you wish to use for adaptivity.

## Example Input File Syntax

!listing /tests/neutronics/indicators/rel_err/openmc.i
  block=Indicators

!syntax parameters /Adaptivity/Indicators/StatRelErrorIndicator

!syntax inputs /Adaptivity/Indicators/StatRelErrorIndicator
