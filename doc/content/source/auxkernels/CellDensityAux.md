# CellDensityAux

!syntax description /AuxKernels/CellDensityAux

## Description

Displays the densities (kg/m$^3$) set in the OpenMC model by MOOSE. If a MOOSE element is not providing density feedback or did not map at all to an OpenMC cell,
then this auxiliary kernel returns $-1$.

## Example Input Syntax

As an example, the `cell_density` auxiliary kernel will extract the OpenMC cell density
and map it to the MOOSE elements corresponding to each OpenMC cell.

!listing test/tests/neutronics/feedback/lattice/openmc.i
  block=AuxKernels

!syntax parameters /AuxKernels/CellDensityAux

!syntax inputs /AuxKernels/CellDensityAux

!syntax children /AuxKernels/CellDensityAux
