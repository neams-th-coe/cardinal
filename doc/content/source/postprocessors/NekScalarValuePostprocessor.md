# NekScalarValuePostprocessor

!syntax description /Postprocessors/NekScalarValuePostprocessor

## Description

This postprocessor will display the value of a [NekScalarValue](NekScalarValue.md),
for diagnostic purposes.

## Example Input Syntax

As an example, the `s1` postprocessor will grab the value held by the
`scalar1` userobject.

!listing test/tests/nek_stochastic/nek_multi.i
  start=UserObjects
  end=Outputs

!syntax parameters /Postprocessors/NekScalarValuePostprocessor

!syntax inputs /Postprocessors/NekScalarValuePostprocessor
