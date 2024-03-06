# OpenMCCoupledCells

!syntax description /Postprocessors/OpenMCCoupledCells

## Description

This postprocessor displays the number of OpenMC cells which are either (i) receiving temperature feedback from MOOSE, (ii) receiving density feedback from MOOSE, or (iii) are writing a cell tally into MOOSE.

!alert note
This postprocessor will only report the number of cells on the specified `cell_level` for coulping. For instance, if you are applying a single temperature to a TRISO fuel pebble, this postprocessor would report that 1 cell is being coupled (the universe filled by the TRISO pebble), and NOT the 10,000+ cells comprising that universe (even though they will receive feedback).

## Example Input Syntax

Shown below is an example for depicting the number of OpenMC cells coupled to MOOSE.

!listing test/tests/postprocessors/coupled_cells/openmc.i
  block=Postprocessors

!syntax parameters /Postprocessors/OpenMCCoupledCells

!syntax inputs /Postprocessors/OpenMCCoupledCells

!syntax children /Postprocessors/OpenMCCoupledCells
