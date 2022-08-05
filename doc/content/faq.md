# FAQ

On this page we collect a number of commonly-asked questions with
possible solutions. If you have a question that isn't covered here,
please post to our [Discussions page](https://github.com/neams-th-coe/cardinal/discussions)
or reach our to a developer on our Slack channels
on the MOOSE Developers Slack workspace
(`moosedevelopers.slack.com`) or on the NekRS Developers Slack workspace
(`nekrsdev-team.slack.com`).

## Errors with PETSc/libMesh

This section addresses errors commonly encountered when compiling Cardinal
which are related to PETSc or libMesh. If you run into problems when building/using
PETSc or libMesh, the issue is almost always at the MOOSE level, and not specific to
Cardinal. We recommend posting your problem directly to the
[MOOSE discussions page](https://github.com/idaholab/moose/discussions)
or to the MOOSE Slack.

!alert! note
If you are trying to use PETSc and libMesh from MOOSE's conda environment, you should
*not* try to later switch to using the script approach
(`contrib/moose/scripts/update_and_rebuild_petsc|libmesh.sh`) because the conda packages
separately export environment variables that will conflict with your application in the
end, if you are trying to provide it with multiple locations of PETSc/libMesh. If you
do want to build PETSc and libMesh from the scripts, you should either:

- Turn off the conda environment and just use your system's compilers and libraries
- Only install the `moose-mpich` conda package, or equivalently slim conda package that
  does not include PETSc/libMesh

!alert-end!

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
