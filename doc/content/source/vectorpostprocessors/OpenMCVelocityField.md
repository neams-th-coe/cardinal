# OpenMCVelocityField

!syntax description /VectorPostprocessors/OpenMCVelocityField

## Description

The `OpenMCVelocityField` vector post-processor retrieves the velocity field values
of an OpenMC model. It provides access to the values of the OpenMC variable
`simulation::velocity_field`, following the same cell ordering used internally
by OpenMC for its velocity field.

## Example Input Syntax

Here is an example of using this post-processor for outputting the values of the OpenMC
velocity field:

!listing test/tests/transfers/openmc_velocity_field/openmc.i
  block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/OpenMCVelocityField

!syntax inputs /VectorPostprocessors/OpenMCVelocityField
