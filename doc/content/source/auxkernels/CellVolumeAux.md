# CellVolumeAux

!syntax description /AuxKernels/CellVolumeAux

## Description

Displays the OpenMC cell mapped volumes (mesh units) on the mesh mirror. This auxkernel
has two options:

- `volume_type = mapped` will display the magnitude of the mesh element
  volumes that each OpenMC cell maps to. So, if you have an sphere OpenMC cell with a
  volume of 1.0 that maps to a set of MOOSE mesh elements with a total volume of 0.95,
  this option will display 0.95 mapped to the `[Mesh]`.
- `volume_type = actual` will display the actual OpenMC cell volumes, computed from
  a stochastic volume calculation. Using the same sphere example, this will then display
  the stochastic estimate of OpenMC's cell volume, which would be something like
  0.98 +/- 0.0021 (with the magnitude of the standard deviation determined by the
  number of samples taken in the stochastic volume calculation). With the `actual` option,
  this auxkernel would display 0.98 mapped to the `[Mesh]`.

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
