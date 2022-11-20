# Troubleshooting the PETSc and libMesh Install

If you run into problems while building PETSc or libMesh, the issue is almost always
at the MOOSE level, and not specific to Cardinal. We recommend posting your problem directly
to the [MOOSE discussions page](https://github.com/idaholab/moose/discussions) or to
the MOOSE Slack. Here, we also collect a number of common issues.

## Mixing conda environment with the scripts

If you are trying to use PETSc and libMesh from MOOSE's conda environment, you should
*not* try to later switch to using the script approach
(`contrib/moose/scripts/update_and_rebuild_petsc|libmesh.sh`) because the conda packages
separately export environment variables that will conflict with your application in the
end, if you are trying to provide it with multiple locations of PETSc/libMesh. If you
do want to build PETSc and libMesh from the scripts, you should either:

- Turn off the conda environment and just use your system's compilers and libraries
- Only install the `moose-mpich` conda package, or equivalently slim conda package that
  does not include PETSc/libMesh
