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
--------------------------------------------------------------------------------------------------------
| Slot |  Data   |      How to Access (.oudf)      |               How to Access (.udf)                |
--------------------------------------------------------------------------------------------------------
|    0 | scalar1 | bc->usrwrk[0*bc->fieldOffset+0] | platform->app->bc->o_usrwrk[0*nrs->fieldOffset+0] |
|    0 | scalar2 | bc->usrwrk[0*bc->fieldOffset+1] | platform->app->bc->o_usrwrk[0*nrs->fieldOffset+1] |
--------------------------------------------------------------------------------------------------------
```

Suppose we want to use these two scalar values to set a Dirichlet temperature boundary
condition in NekRS. We would simply follow the instructions for the `.oudf`, and do:

!listing test/tests/transfers/nek_scalar_value/pyramid.oudf language=cpp

!syntax parameters /Problem/ScalarTransfers/NekScalarValue

!syntax inputs /Problem/ScalarTransfers/NekScalarValue
