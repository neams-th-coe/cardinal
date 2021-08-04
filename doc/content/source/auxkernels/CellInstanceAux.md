# CellInstanceAux

!syntax description /AuxKernels/CellInstanceAux

## Description

Displays the OpenMC cell instance mapped to the MOOSE elements. If there are
no distributed cells in your OpenMC geometry, then the instance for every cell
is zero. But if you have distributed cells such as for problems with lattices
or if you repeat the same universe multiple places throughout the geometry,
then this auxiliary kernel
can be used to visualize the cell instance that
maps to each MOOSE element.

If a MOOSE element did not map at all to an OpenMC cell,
then this auxiliary kernel returns $-1$.

## Example Input Syntax

As an example, the `cell_instance` auxiliary kernel will extract the OpenMC cell instance
and map it to the MOOSE elements corresponding to each OpenMC cell.

!listing test/tests/neutronics/feedback/lattice/openmc.i
  block=AuxKernels

This particular OpenMC geometry repeats the same universe several times throughout
the geometry, so the instance for a particular cell will range from 0 to $N-1$, where
$N$ is the number of times the cell is repeated through the geometry.

!syntax parameters /AuxKernels/CellInstanceAux

!syntax inputs /AuxKernels/CellInstanceAux

!syntax children /AuxKernels/CellInstanceAux
