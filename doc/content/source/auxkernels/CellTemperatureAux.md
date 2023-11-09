# CellTemperatureAux

!syntax description /AuxKernels/CellTemperatureAux

## Description

Displays the OpenMC cell temperature (K) mapped to the MOOSE elements, showing all cells
which either (i) receive multiphysics feedback or (ii) send a cell tally to MOOSE.

Note that this temperature is not necessarily the temperature at which cross
sections are evaluated at. For more information, see the `temperature`-related
elements in [OpenMC's settings](https://docs.openmc.org/en/latest/io_formats/settings.html#temperature-default-element).

If a MOOSE element did not map at all to an OpenMC cell,
then this auxiliary kernel returns $-1$.

## Example Input Syntax

As an example, the `cell_temperature` auxiliary kernel will extract the OpenMC cell temperature
and map it to the MOOSE elements corresponding to each OpenMC cell.

!listing test/tests/neutronics/feedback/lattice/openmc.i
  block=AuxKernels

!syntax parameters /AuxKernels/CellTemperatureAux

!syntax inputs /AuxKernels/CellTemperatureAux

!syntax children /AuxKernels/CellTemperatureAux
