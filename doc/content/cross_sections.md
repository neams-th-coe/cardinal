# Getting Cross Section Data

OpenMC requires cross section data in HDF5 format.

## Test Suite

Cardinal's test suite assumes that you are using a specific data library
that you can obtain by running

```
./scripts/download-test-suite-cross-sections.sh
```

This script will place cross section data into a directory
name `test_suite_cross_sections` one directory "above" the Cardinal repository.

!alert warning
We do *not* recommend using this data set for analysis. It is known to have some
minor incorrect behavior for temperature-dependent data due to a previous
[bug in NJOY](https://github.com/openmc-dev/data/issues/77)
that we have since fixed (but this data was assumed in hundreds of
our software tests). To guarantee code correctness, we therefore opted to not
change the data set assumed in the test suite, since the purpose of the test suite
is to verify software quality and backwards compatibility, not necessarily the
physics correctness.

## Analysis

For routine analysis, please use one of the official libraries maintained by the OpenMC
team. To quickly fetch the ENDF-B7.1 data set, you can run

```
./scripts/download-openmc-cross-sections.sh
```

This script will place the cross section data into a directory named
`cross_sections` one directory "above" the Cardinal repository.
If you would prefer other libraries, please refer to the
[OpenMC cross section documentation](https://docs.openmc.org/en/stable/usersguide/cross_sections.html).
