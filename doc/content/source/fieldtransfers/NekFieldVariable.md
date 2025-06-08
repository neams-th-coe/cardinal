# NekFieldVariable

## Description

The `NekFieldVariable` class sends a field variable between NekRS and MOOSE.
A field variable is any of the solution fields computed by NekRS, such as temperature,
pressure, velocity, and other passive scalars. First, this object
creates an [AuxVariable](AuxVariable.md) using the name of the object; this variable will hold
the variable which NekRS reads (for `direction = to_nek`) or will be written by NekRS (for
`direction = from_nek`). If not provided, the name of the auxiliary variable will be
taken as the name of the object, if it can be matched to one of the enumerations for
the `field`.

For `direction = from_nek`, this class provides an extremely useful manner to obtain
the NekRS solution field in MOOSE.
For instance, [output_sfr] shows the velocity from the NekRS field files
(left) and interpolated onto a second-order mesh mirror (right) using a `NekFieldVariable`. Because this particular
example runs NekRS in a higher order than can be represented on a second-order mesh
mirror, the interpolated velocity is clearly not an exact representation of the NekRS
solution - only an interpolated version of the NekRS solution.

!media output_vel.png
  id=output_sfr
  caption=Velocity from the NekRS field files (left) and after interpolation onto a second order mesh mirror (right).
  style=width:80%;margin-left:auto;margin-right:auto;halign:center

## Example Input File Syntax

As an example, the example below reads the spectral element temperature field from NekRS
and writes it into an [AuxVariable](AuxVariable.md) named `temp`.

!listing tests/nek_temp/first_order/nek.i
  block = Problem

!syntax parameters /Problem/FieldTransfers/NekFieldVariable

!syntax inputs /Problem/FieldTransfers/NekFieldVariable
