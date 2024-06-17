If you would like to check that Cardinal was built correctly and that you
have all the basic requirements in place, we can walk you through a few
installation checks and try running a few input files.
If you run into any issues with the following commands, you can find an FAQ of common
issues [here](faq.md).

1. If using OpenMC, make sure that you have cross sections downloaded. If the following returns an empty line, you need to [download cross sections](cross_sections.md).

```
echo $OPENMC_CROSS_SECTIONS
```

2. If using OpenMC, try running a multiphysics case.

```
cd test/tests/neutronics/feedback/lattice
mpiexec -np 2 ../../../../../cardinal-opt -i openmc_master.i --n-threads=2
```

3. If using OpenMC, try building the OpenMC XML files using OpenMC's Python API. If you run into any issues, you most likely need to [install OpenMC's Python API](python_api.md).

```
cd tutorials/lwr_solid
python make_openmc_model.py
```

4. If using NekRS, try running a conjugate heat transfer case.

```
cd test/tests/cht/sfr_pincell
mpiexec -np 4 ../../../../cardinal-opt -i nek_master.i
```

5. Try leveraging NekRS's tools to make a mesh. If you run into any issues, you most likely need to [install the NekRS tools](nek_tools.md).

```
cd test/tests/conduction/boundary_and_volume/prism
exo2nek
```

### For Developers

You can run Cardinal's regression test suite with the following:

```
./run_tests -j8
```

which will run the tests in parallel with 8 processes. OpenMC's tests require
you to use [this data set](cross_sections.md).
Depending on the availability of various dependencies, some tests may be skipped.
The first time
you run the test suite, the runtime will be very long due to the just-in-time compilation of
NekRS. Subsequent runs will be much faster due to the use of cached build files.
If you run into issues running the test suite, please check out our
[run_tests troubleshooting page](run_tests_troubleshoot.md).

You can run the unit tests with the following:

```
cd unit
make -j8
./run_tests -j8
```
