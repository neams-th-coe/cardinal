# NekFieldVariable

## Description

`NekFieldVariable` is a [FieldTransfer](AddFieldTransferAction.md) that sends a field variable between NekRS and MOOSE.
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

The following input
sections will interpolate the NekRS pressure, velocity, and temperature for
the `lowMach` NekRS example
onto a second order representation of the NekRS mesh.

!listing test/tests/nek_standalone/lowMach/nek.i
  block=Problem

Then, the output format specified in the `[Output]` block will be used. Here,
the NekRS solution is projected onto an Exodus mesh.

!listing
[Outputs]
  exodus = true
[]

For instance, [output_p] shows the pressure from the NekRS field files (left)
and interpolated onto a second-order mesh mirror (right). Because this particular
example runs NekRS in a higher order than can be represented on a second-order
mesh mirror, the interpolated pressure is clearly not an exact
representation of the NekRS solution - only an interpolated version of the NekRS solution.

!media output_p.png
  id=output_p
  caption=Pressure from the NekRS field files (left) and after interpolation onto a second order mesh mirror (right).
  style=width:90%;margin-left:auto;margin-right:auto;halign:center

!syntax parameters /Problem/FieldTransfers/NekFieldVariable

!syntax inputs /Problem/FieldTransfers/NekFieldVariable
