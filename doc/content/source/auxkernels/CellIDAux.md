# CellIDAux

!syntax description /AuxKernels/CellIDAux

## Description

Displays the OpenMC cell ID mapped to the MOOSE elements which provide
multiphysics feedback *into* OpenMC.
In other words, this auxiliary kernel can be used to visualize the cell that
maps to each MOOSE element for receiving temperature and/or density feedback.

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
