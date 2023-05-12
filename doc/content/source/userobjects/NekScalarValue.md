# NekScalarValue

!syntax description /UserObjects/NekScalarValue

## Description

This user object sends a scalar value from MOOSE into NekRS's scratch space.
This can be used to control NekRS simulations using MOOSE's
[Controls](https://mooseframework.inl.gov/syntax/Controls/index.html) system
or stochastically perturb NekRS simulations using the
[MOOSE stochastic tools module](https://mooseframework.inl.gov/modules/stochastic_tools/index.html).

## Example Input Syntax

Below is an example input file that defines two scalar user objects that we will use
to control a NekRS boundary condition from MOOSE using the `Controls` system.

!listing test/tests/userobjects/nek_scalar_value/nek.i
  block=UserObjects

We will control the `value` in each of these parameters using a
[RealFunctionControl](https://mooseframework.inl.gov/source/controls/RealFunctionControl.html),
a MOOSE object that lets us define a scalar value using a function. Here, we will use a simple
constant value function, which changes value partway through the simulation.

!listing test/tests/userobjects/nek_scalar_value/nek.i
  start=Controls
  end=Postprocessors

When running the input file, you will then see a table like the following print:

```
 ===================>     MAPPING FROM MOOSE TO NEKRS      <===================

          Slice:  entry in NekRS scratch space
       Quantity:  physical meaning or name of data in this slice
  How to Access:  C++ code to use in NekRS files; for the .udf instructions,
                  'n' indicates a loop variable over GLL points

------------------------------------------------------------------------------------------------
| Quantity |           How to Access (.oudf)           |         How to Access (.udf)          |
------------------------------------------------------------------------------------------------
| scalar1  | bc->usrwrk[0 * bc->fieldOffset + 0]       | nrs->usrwrk[0 * nrs->fieldOffset + 0] |
| scalar2  | bc->usrwrk[0 * bc->fieldOffset + 1]       | nrs->usrwrk[0 * nrs->fieldOffset + 1] |
| unused   | bc->usrwrk[1 * bc->fieldOffset + bc->idM] | nrs->usrwrk[1 * nrs->fieldOffset + n] |
| unused   | bc->usrwrk[2 * bc->fieldOffset + bc->idM] | nrs->usrwrk[2 * nrs->fieldOffset + n] |
| unused   | bc->usrwrk[3 * bc->fieldOffset + bc->idM] | nrs->usrwrk[3 * nrs->fieldOffset + n] |
| unused   | bc->usrwrk[4 * bc->fieldOffset + bc->idM] | nrs->usrwrk[4 * nrs->fieldOffset + n] |
| unused   | bc->usrwrk[5 * bc->fieldOffset + bc->idM] | nrs->usrwrk[5 * nrs->fieldOffset + n] |
| unused   | bc->usrwrk[6 * bc->fieldOffset + bc->idM] | nrs->usrwrk[6 * nrs->fieldOffset + n] |
------------------------------------------------------------------------------------------------
```

Suppose we want to use these two scalar values to set a Dirichlet temperature boundary
condition in NekRS. We would simply follow the instructions for the `.oudf`, and do:

!listing test/tests/userobjects/nek_scalar_value/pyramid.oudf language=cpp

!syntax parameters /UserObjects/NekScalarValue

!syntax inputs /UserObjects/NekScalarValue

!syntax children /UserObjects/NekScalarValue
