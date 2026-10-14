# OpenMCTemperatureField

!syntax description /VectorPostprocessors/OpenMCTemperatureField

## Description

The `OpenMCTemperatureField` vector post-processor retrieves the temperature field values
of an OpenMC model. It provides access to the values of the OpenMC variable
`simulation::temperature_field`, following the same cell ordering used internally
by OpenMC for its temperature field.

## Example Input Syntax

Here is an example of using this post-processor for outputting the values of the OpenMC
temperature field:

!listing test/tests/transfers/openmc_temperature_field/openmc.i
  block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/OpenMCTemperatureField

!syntax inputs /VectorPostprocessors/OpenMCTemperatureField
