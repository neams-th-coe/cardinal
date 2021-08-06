# CellTemperatureAux

!syntax description /AuxKernels/CellTemperatureAux

## Description

Displays the OpenMC cell temperature (K) mapped to the MOOSE elements.
In other words, this auxiliary kernel can be used to visualize the temperature that
gets set on the OpenMC cells based on the MOOSE elements that map to each cell.

Note that this temperature is not necessarily the temperature at which cross
sections are evaluated at - the `temperature_method` element in OpenMC's
`settings.xml` file is either one of "nearest" or "interpolation". That is, if
you have set the temperature method in OpenMC to nearest, then you will actually
evaluate cross sections at the data set closest to the temperature shown by this
auxiliary kernel. If you instead set the temperature method in OpenMC to interpolation,
then you will actually evaluate cross sections as a stochastic interpolation
between the two nearest data sets.

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
