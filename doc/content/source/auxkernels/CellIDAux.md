# CellIDAux

!syntax description /AuxKernels/CellIDAux

## Description

Displays the OpenMC cell ID mapped to the MOOSE elements, showing all cells
which either (i) receive multiphysics feedback or (ii) send a cell tally to MOOSE.
This auxkernel is automatically added to all OpenMC wrappings which involve
some sort of multiphysics feedback to cells and/or cell tallies, so you do not need
to add it yourself. The output variable is named `cell_id`.

If a MOOSE element did not map at all to an OpenMC cell,
then this auxiliary kernel returns $-1$.

## Example Input Syntax

As an example, the `cell_id` auxiliary kernel will display the OpenMC cell ID
corresponding to sets of MOOSE elements.

!listing test/tests/neutronics/feedback/lattice/openmc.i
  block=AuxKernels

For this particular input, the OpenMC geometry repeats the same universe
several times throughout the geometry; therefore, the same cell ID is repeated
several times in the geometry.

!syntax parameters /AuxKernels/CellIDAux

!syntax inputs /AuxKernels/CellIDAux

!syntax children /AuxKernels/CellIDAux
