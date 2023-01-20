If you would like to check that Cardinal was built correctly and that you
have all the basic requirements in place, you can try running a few input files.

1. If you are using OpenMC, try:

```
cd test/tests/neutronics/feedback/lattice
mpiexec -np 2 ../../../../../cardinal-opt -i openmc_master.i --n-threads=2
```

If you run into any issues when running OpenMC cases, check out our
[troubleshooting OpenMC at runtime](openmc_runtime.md) page.

2. If you are using OpenMC and also want to leverage OpenMC's
   [Python API](https://docs.openmc.org/en/stable/usersguide/install.html#installing-python-api)
   to make OpenMC models, try:

```
cd tutorials/lwr_solid
python make_openmc_model.py
```

3. If you are using NekRS, try:

```
cd test/tests/cht/sfr_pincell
mpiexec -np 4 ../../../../cardinal-opt -i nek_master.i
```

If you run into any issues when running NekRS cases, check out our
[troubleshooting NekRS at runtime](nekrs_runtime.md) page.

4. If you are using NekRS and also want to leverage [NekRS's tools](nek_tools.md)
   to make meshes, view output files in Paraview, etc., try:

```
cd test/tests/conduction/boundary_and_volume/prism
exo2nek
```

5. For developers, you will also find it useful to run Cardinal's
   test suite, which consists of unit and regression tests.
   You can run Cardinal's regression test suite with the following:

```
./run_tests -j8
```

which will run the tests in parallel with 8 processes. OpenMC's tests require
you to use the ENDF/B7-II.1 data set, which you can download following
the instructions [here](cross_sections.md).
Depending on the availability of various dependencies, some tests may be skipped.
The first time
you run the test suite, the runtime will be very long due to the just-in-time compilation of
NekRS. Subsequent runs will be much faster due to the use of cached build files.

You can run the unit tests with the following:

```
cd unit
make -j8
./run_tests -j8
```

If you run into issues running the test suite, please check out our
[run_tests troubleshooting page](run_tests_troubleshoot.md).
