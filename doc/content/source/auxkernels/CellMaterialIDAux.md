# CellMaterialIDAux

!syntax description /AuxKernels/CellMaterialIDAux

## Description

Displays the OpenMC fluid material ID mapped to the MOOSE elements
denoted as fluid by [OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md).
In other words, this auxiliary kernel can be used to visualize the material ID that
maps to each MOOSE element.

If a MOOSE element is not fluid or did not map at all to an OpenMC cell,
then this auxiliary kernel returns $-1$.

## Example Input Syntax

As an example, the `material_id` auxiliary kernel will extract the OpenMC material ID
and map it to the MOOSE elements corresponding to each OpenMC cell.

!listing test/tests/neutronics/feedback/lattice/openmc.i
  block=AuxKernels

!syntax parameters /AuxKernels/CellMaterialIDAux

!syntax inputs /AuxKernels/CellMaterialIDAux

!syntax children /AuxKernels/CellMaterialIDAux
