# CellTemperatureAux

!syntax description /AuxKernels/CellTemperatureAux

## Description

Displays the temperatures (K) set in the OpenMC model by MOOSE. If a MOOSE element is not providing temperature feedback or did not map at all to an OpenMC cell,
then this auxiliary kernel returns $-1$.

Note that this temperature is not necessarily the temperature at which cross
sections are evaluated at. For more information, see the `temperature`-related
elements in [OpenMC's settings](https://docs.openmc.org/en/latest/io_formats/settings.html#temperature-default-element).

If a MOOSE element did not map at all to an OpenMC cell or if an element does not provide feedback,
then this auxiliary kernel returns $-1$.

## Example Input Syntax

As an example, the `cell_temperature` auxiliary kernel will extract the OpenMC cell temperature
and map it to the MOOSE elements corresponding to each OpenMC cell.

!listing test/tests/neutronics/feedback/lattice/openmc.i
  block=AuxKernels

!syntax parameters /AuxKernels/CellTemperatureAux

!syntax inputs /AuxKernels/CellTemperatureAux

!syntax children /AuxKernels/CellTemperatureAux
