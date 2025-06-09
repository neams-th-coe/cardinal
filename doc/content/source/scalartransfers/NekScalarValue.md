# NekScalarValue

## Description

The `NekScalarValue` class sends a scalar value between NekRS and MOOSE.
This can be used to control NekRS simulations using MOOSE's
[Controls](Controls/index.md) system
or stochastically perturb NekRS simulations using the
[MOOSE stochastic tools module](stochastic_tools/index.md). Passing a number
can also be used for much more general purposes (anywhere that a scalar value
appears in NekRS's `.udf` or `.oudf` files).

## Example Input File Syntax

Below is an example input file that defines two scalar transfers that we will use
to control a NekRS boundary condition from MOOSE using the `Controls` system.

!listing test/tests/transfers/nek_scalar_value/nek.i
  block=Problem

We will control the `value` in each of these parameters using a
[RealFunctionControl](RealFunctionControl.md),
a MOOSE object that lets us define a scalar value using a function. Here, we will use a simple
constant value function, which changes value partway through the simulation.

!listing test/tests/transfers/nek_scalar_value/nek.i
  start=Controls
  end=Postprocessors

When running the input file, you will then see a table like the following print:

```
-------------------
| Slot | MOOSE quantity |          How to Access (.oudf)          |         How to Access (.udf)          |
-----------------------------------------------------------------------------------------------------------
|    0 | scalar1        | bc->usrwrk[0 * bc->fieldOffset + 0]     | nrs->usrwrk[0 * nrs->fieldOffset + 0] |
|    0 | scalar2        | bc->usrwrk[0 * bc->fieldOffset + 1]     | nrs->usrwrk[0 * nrs->fieldOffset + 1] |
|    1 | unused         | bc->usrwrk[1 * bc->fieldOffset+bc->idM] | nrs->usrwrk[1 * nrs->fieldOffset + n] |
|    2 | unused         | bc->usrwrk[2 * bc->fieldOffset+bc->idM] | nrs->usrwrk[2 * nrs->fieldOffset + n] |
|    3 | unused         | bc->usrwrk[3 * bc->fieldOffset+bc->idM] | nrs->usrwrk[3 * nrs->fieldOffset + n] |
|    4 | unused         | bc->usrwrk[4 * bc->fieldOffset+bc->idM] | nrs->usrwrk[4 * nrs->fieldOffset + n] |
|    5 | unused         | bc->usrwrk[5 * bc->fieldOffset+bc->idM] | nrs->usrwrk[5 * nrs->fieldOffset + n] |
|    6 | unused         | bc->usrwrk[6 * bc->fieldOffset+bc->idM] | nrs->usrwrk[6 * nrs->fieldOffset + n] |
-----------------------------------------------------------------------------------------------------------
```

Suppose we want to use these two scalar values to set a Dirichlet temperature boundary
condition in NekRS. We would simply follow the instructions for the `.oudf`, and do:

!listing test/tests/transfers/nek_scalar_value/pyramid.oudf language=cpp

!syntax parameters /Problem/ScalarTransfers/NekScalarValue

!syntax inputs /Problem/ScalarTransfers/NekScalarValue
