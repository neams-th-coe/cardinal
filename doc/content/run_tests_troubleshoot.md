# Troubleshooting the Test Suite

This page addresses errors commonly encountered when running the regression
test suite, with `./run_tests`.

## All the OpenMC tests Exodiff/CSVDiff

There are two general reasons why all of the OpenMC tests might fail with
an Exodiff or CSVDiff (meaning that the test *ran*, but the code results
do not match what the test suite expects to encounter for tally values):

- You did not use the cross section set that Cardinal expects to use for the test suite.
  You can find this test suite [here](https://cardinal.cels.anl.gov/cross_sections.html).
- If you are on a Mac, it is known that OpenMC itself does not output identical solutions
  when compared to Linux operating systems. See [this issue](https://github.com/openmc-dev/openmc/issues/801)
  on the OpenMC github for more information. In this case, as long as the tests ran
  (i.e. they did not crash or abort, but did run successfully), you can proceed as normal.

## Missing Python dependencies

Cardinal's test suite requires some Python dependencies that ship with MOOSE. You
might encounter an error like:

```
Traceback (most recent call last):
  File "/Users/anovak/projects/cardinal/./run_tests", line 11, in <module>
    from TestHarness import TestHarness
ModuleNotFoundError: No module named 'TestHarness'
```

This means that you need to:

- Be sure that your Python is version 3 (`python --version` will show you the version)
- Add `cardinal/contrib/moose/python` to your `PYTHONPATH` to be sure Python can find `TestHarness`

## Missing libmesh-config

If you encounter an error like

```
Error! Could not find 'libmesh-config' in any of the usual libmesh's locations!
```

This means that you need to explicitly set `LIBMESH_DIR` to point
to where you have `moose/libmesh`. If you're using MOOSE's conda environment, this
means setting `LIBMESH_DIR` to `$CONDA_PREFIX/libmesh`. If not using the conda
environment, set to `cardinal/contrib/moose/libmesh/installed`. You may also need to explicitly
set `MOOSE_DIR` to point to `cardinal/contrib/moose`.

## Timeout NekRS tests

The first time that you run a NekRS input file, NekRS does just-in-time compilation
of the OCCA kernels, which can be *very slow*. MOOSE's default timeout for a test is
a runtime of 300 seconds. If your system does not have very many cores, a lot of the
NekRS-related tests can fail the first time you run the test suite. Just try re-running
again, in which case NekRS will be able to re-use the build files you created during
the first pass so that the tests are much faster.

## Crashing NekRS tests

NekRS is known to randomly fail to precompile tests; see for example
[this old issue](https://github.com/Nek5000/nekRS/issues/166). It is rare for these failures
to happen when running a simulation using `cardinal-opt` directly, but for some reason
they can be much more common when running a case through MOOSE's test harness via
`./run_tests`. You may encounter random "crash" type test failures with error messages
related to "undefined symbols" or "dlopen" failures, to give a
few examples. As long as you can run the test directly with `cardinal-opt`, these failures
do not imapct the code's correctness or usability.
We are diagnosing these issues longer-term.
