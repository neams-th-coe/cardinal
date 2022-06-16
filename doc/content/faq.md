# FAQ

On this page we collect a number of commonly-asked questions with
possible solutions. If you have a question that isn't covered here,
please post to our [Discussions page](https://github.com/neams-th-coe/cardinal/discussions)
or reach our to a developer on our Slack channels
on the MOOSE Developers Slack workspace
(`moosedevelopers.slack.com`) or on the NekRS Developers Slack workspace
(`nekrsdev-team.slack.com`).

## Errors with Test Suite

This section addresses errors commonly encountered when running the regression
test suite, with `./run_tests`.

!alert! note
```
Traceback (most recent call last):
  File "/Users/anovak/projects/cardinal/./run_tests", line 11, in <module>
    from TestHarness import TestHarness
ModuleNotFoundError: No module named 'TestHarness'
```

This means that you need to:

- Be sure that your Python is version 3 (`python --version` will show you the version)
- Add `cardinal/contrib/moose/python` to your `PYTHONPATH` to be sure Python can find `TestHarness`

!alert-end!

!alert! note
```
Error! Could not find 'libmesh-config' in any of the usual libmesh's locations!
```

This means that you need to explicitly set `LIBMESH_DIR` to point
to where you have `moose/libmesh`. If you're using MOOSE's conda environment, this
means setting `LIBMESH_DIR` to `$CONDA_PREFIX/libmesh`. If not using the conda
environment, set to `cardinal/contrib/moose/libmesh`.

!alert-end!
