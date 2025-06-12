# NekPostprocessorValue

## Description

The `NekPostprocessorValue` class sends a postprocessor value between NekRS and MOOSE.
This can be used to apply boundary conditions to the NekRS model, as well as the much
more general purposes of setting values that appear anywhere in NekRS's `.udf` or `.oudf` files. This object will automatically create a [Receiver](Receiver.md) postprocessor
with the name of the object; data to read/write will be taken/placed in this postprocessor.

## Example Input File Syntax

Below is an example input file that defines two postprocessor transfers that we will use
to set two boundary conditions in NekRS. This also creates automatically two
[Receiver](Receiver.md) postprocessors, named `inlet_V` and `inlet_T`.

!listing test/tests/transfers/nek_postprocessor_value/nek.i
  block=Problem

A sub-application sends data into the `inlet_V` and `inlet_T` postprocessors.
When running the input file, you will then see a table like the following print:

```
-----------------------------------------------------------------------------------------------------------
| Slot | MOOSE quantity |          How to Access (.oudf)          |         How to Access (.udf)          |
-----------------------------------------------------------------------------------------------------------
|    0 | inlet_V        | bc->usrwrk[0 * bc->fieldOffset + 0]     | nrs->usrwrk[0 * nrs->fieldOffset + 0] |
|    1 | inlet_T        | bc->usrwrk[1 * bc->fieldOffset + 0]     | nrs->usrwrk[1 * nrs->fieldOffset + 0] |
|    2 | unused         | bc->usrwrk[2 * bc->fieldOffset+bc->idM] | nrs->usrwrk[2 * nrs->fieldOffset + n] |
|    3 | unused         | bc->usrwrk[3 * bc->fieldOffset+bc->idM] | nrs->usrwrk[3 * nrs->fieldOffset + n] |
|    4 | unused         | bc->usrwrk[4 * bc->fieldOffset+bc->idM] | nrs->usrwrk[4 * nrs->fieldOffset + n] |
|    5 | unused         | bc->usrwrk[5 * bc->fieldOffset+bc->idM] | nrs->usrwrk[5 * nrs->fieldOffset + n] |
|    6 | unused         | bc->usrwrk[6 * bc->fieldOffset+bc->idM] | nrs->usrwrk[6 * nrs->fieldOffset + n] |
-----------------------------------------------------------------------------------------------------------
```

Suppose we want to use these two scalar values to set a Dirichlet temperature
and velocity boundary condition in NekRS. We would simply use those quantities in the
`.oudf` column in the table above.

!listing test/tests/transfers/nek_postprocessor_value/pyramid.oudf language=cpp

!syntax parameters /Problem/ScalarTransfers/NekPostprocessorValue

!syntax inputs /Problem/ScalarTransfers/NekPostprocessorValue
