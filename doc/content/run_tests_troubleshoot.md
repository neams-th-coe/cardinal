# Troubleshooting the Test Suite

This page addresses errors commonly encountered when running the regression
test suite, with `./run_tests`.

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
environment, set to `cardinal/contrib/moose/libmesh`.
