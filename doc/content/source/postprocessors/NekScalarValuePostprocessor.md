# NekScalarValuePostprocessor

!syntax description /Postprocessors/NekScalarValuePostprocessor

## Description

This postprocessor will display the value of a [NekScalarValue](https://cardinal.cels.anl.gov/source/userobjects/NekScalarValue.html),
for diagnostic purposes.

## Example Input Syntax

As an example, the `flux1` and `flux2` postprocessors
below will evaluate the integral of the first slot in `nrs->usrwrk`
over two different boundaries in NekRS's mesh. For conjugate heat transfer,
this particular slot represents the heat flux boundary condition sent to NekRS.

!listing test/tests/conduction/zero_flux/nek_disjoint.i
  block=Postprocessors

!syntax parameters /Postprocessors/NekScalarValuePostprocessor

!syntax inputs /Postprocessors/NekScalarValuePostprocessor

!syntax children /Postprocessors/NekScalarValuePostprocessor
