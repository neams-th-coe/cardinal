# AddScalarTransferAction

## Overview

The `AddScalarTransferAction` is responsible for performing data transfers between NekRS and MOOSE of
scalar values (single numbers).
This is done with the `[ScalarTransfers]` block in a Cardinal input file.
Examples of field transfers which can be added include:

- [NekPostprocessorValue](NekPostprocessorValue.md), to exchange a postprocessor value
- [NekScalarValue](NekScalarValue.md), to exchange a scalar value (a number)

## Usrwrk Array

When `direction = to_nek`, data is "sent" into NekRS by writing into the `nrs->usrwrk` scratch space array and also copying from this array into its corresponding array on the host (`nrs->o_usrwrk`)
so that the data is accessible both on host and device within NekRS. You must indicate
which slot in the array to write the scalar value using `usrwrk_slot`. For memory
efficiency, the same slot can be used for many different `ScalarTransfers`, since
each transfer only needs a single entry (one double number) in the array. An offset
in the array is automatically computed so that different `ScalarTransfers` don't
overwrite one another. Note that Cardinal automatically
allocates the `nrs->usrwrk` array, but you can control the size of it by setting `n_usrwrk_slots`
in [NekRSProblem](NekRSProblem.md).

!include seg_fault_warning.md

## Example Input File Syntax

As an example, two [NekScalarValues](NekScalarValue.md) are added in the `[Problem/ScalarTransfers]` block. These objects will pass
the numbers 2.5 and 3.5 into NekRS. Both write into the second slot in the scratch
space, but due to an automatic offset they will not overwrite each other.

!listing /tests/nek_stochastic/shift/nek.i
  block=Problem

When running this case, Cardinal will print out a table showing a summary of what data is
occupying the various slots in the usrwrk arrays and how they can be accessed. This table
for the above example is listed below, which shows how the two numbers can be accessed.

```
-----------------------------------------------------------------------------------------------------------
| Slot | MOOSE quantity |          How to Access (.oudf)          |         How to Access (.udf)          |
-----------------------------------------------------------------------------------------------------------
|    0 | unused         | bc->usrwrk[0 * bc->fieldOffset+bc->idM] | nrs->usrwrk[0 * nrs->fieldOffset + n] |
|    1 | s2             | bc->usrwrk[1 * bc->fieldOffset + 0]     | nrs->usrwrk[1 * nrs->fieldOffset + 0] |
|    1 | s3             | bc->usrwrk[1 * bc->fieldOffset + 1]     | nrs->usrwrk[1 * nrs->fieldOffset + 1] |
|    2 | unused         | bc->usrwrk[2 * bc->fieldOffset+bc->idM] | nrs->usrwrk[2 * nrs->fieldOffset + n] |
-----------------------------------------------------------------------------------------------------------
```

For instance, this passed data is then used in the `brick.udf` file to set
the values of various passive scalars in NekRS.

!listing /tests/nek_stochastic/shift/brick.udf language=cpp
  re=void\sUDF_ExecuteStep.*?^

!syntax list /Problem/ScalarTransfers actions=false subsystems=false heading=Available ScalarTransfer Objects

!syntax parameters /Problem/ScalarTransfers/AddScalarTransferAction
