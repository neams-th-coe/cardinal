# NekScalarValue

!syntax description /UserObjects/NekScalarValue

## Description

This user object sends a scalar value from MOOSE into NekRS's scratch space.
Among other uses, one example application is coupling NekRS to the MOOSE stochastic tools
module, where MOOSE perturbs scalar values that get consumed in NekRS (such as in
boundary conditions or material properties).

## Example Input Syntax

Below is an example input file that defines two arbitrary scalar user objects.

!listing test/tests/nek_stochastic/nek.i
  block=UserObjects

When running the input file, you will then see a table like the following print:

```
 ===================>     MAPPING FROM MOOSE TO NEKRS      <===================

          Slice:  entry in NekRS scratch space
       Quantity:  physical meaning or name of data in this slice
  How to Access:  C++ code to use in NekRS files; for the .udf instructions,
                  'n' indicates a loop variable over GLL points

---------------------------------------------------------------------------------------------
| Quantity |          How to Access (.oudf)         |         How to Access (.udf)          |
---------------------------------------------------------------------------------------------
| scalar1  | bc->wrk[0 * bc->fieldOffset + 0]       | nrs->usrwrk[0 * nrs->fieldOffset + 0] |
| scalar2  | bc->wrk[0 * bc->fieldOffset + 1]       | nrs->usrwrk[0 * nrs->fieldOffset + 1] |
| unused   | bc->wrk[1 * bc->fieldOffset + bc->idM] | nrs->usrwrk[1 * nrs->fieldOffset + n] |
---------------------------------------------------------------------------------------------
```

Then, if you want to use the `value` set in the `scalar1` postprocessor in the `.oudf` file,
you would access as `bc->wrk[0 * bc->fieldOffset + 0]`. For example, if the `scalar1` object
held a value that represented an inlet temperature, you could apply that as a BC like:

```
void scalarDirichletConditions(bcData * bc)
{
  bc->s = bc->wrk[0 * bc->fieldOffset + 0];
}
```

!syntax parameters /UserObjects/NekScalarValue

!syntax inputs /UserObjects/NekScalarValue

!syntax children /UserObjects/NekScalarValue
