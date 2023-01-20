# CellVolumeAux

!syntax description /AuxKernels/CellVolumeAux

## Description

Displays the OpenMC cell mapped volumes (mesh units) on the mesh mirror.
In other words, this auxiliary kernel can be used to visualize the magnitude of the volume
that each OpenMC cell gets mapped to. This can be used to verify visually the volumes
that each OpenMC cell maps to. Note that this auxiliary kernel does *not* display
the actual volume of the OpenMC cells, but rather only the element volume
that those cells map to.

If a MOOSE element did not map at all to an OpenMC cell,
then this auxiliary kernel returns $-1$.

## Example Input Syntax

As an example, the `cell_volume` auxiliary kernel will extract the mapped OpenMC
cell volume
and display it on the MOOSE elements corresponding to each OpenMC cell.

!listing test/tests/neutronics/heat_source/cell_volumes.i
  block=AuxKernels

!syntax parameters /AuxKernels/CellVolumeAux

!syntax inputs /AuxKernels/CellVolumeAux

!syntax children /AuxKernels/CellVolumeAux
