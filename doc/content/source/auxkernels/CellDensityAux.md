# CellDensityAux

!syntax description /AuxKernels/CellDensityAux

## Description

Displays the OpenMC fluid density (kg/m$^3$) mapped to the MOOSE elements
denoted as fluid by [OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md).
In other words, this auxiliary kernel can be used to visualize the density that
gets set on the OpenMC cells based on the MOOSE elements that map to each cell.

If a MOOSE element is not fluid or did not map at all to an OpenMC cell,
then this auxiliary kernel returns $-1$.

## Example Input Syntax

As an example, the `cell_density` auxiliary kernel will extract the OpenMC cell density
and map it to the MOOSE elements corresponding to each OpenMC cell.

!listing test/tests/neutronics/feedback/lattice/openmc.i
  block=AuxKernels

!syntax parameters /AuxKernels/CellDensityAux

!syntax inputs /AuxKernels/CellDensityAux

!syntax children /AuxKernels/CellDensityAux
