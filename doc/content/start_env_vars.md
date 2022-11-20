A number of environment variables are required or recommended when building/running Cardinal.
Put these in your `~/.bashrc`:

``` language=bash
# [REQUIRED] you must set the location of the root directory of the NekRS install;
# this will be the 'install' directory at the top level of the Cardinal repository.
export NEKRS_HOME=$HOME/cardinal/install

# [REQUIRED IF USING MOOSE CONDA ENV for HDF5] you must set the location of the
# root HDF5 directory provided by MOOSE for OpenMC to find
export HDF5_ROOT=$CONDA_PREFIX

# [OPTIONAL] it's a good idea to explicitly note that you are using MPI compiler wrappers
export CC=mpicc
export CXX=mpicxx
export FC=mpif90

# [OPTIONAL] if running with OpenMC, you will need cross section data at runtime;
# you will need to set this variable to point to a 'cross_sections.xml' file.
export OPENMC_CROSS_SECTIONS=${HOME}/cross_sections/endfb71_hdf5/cross_sections.xml
```

!alert! tip title=Additional environment variables
For even further control, you can set other
[optional environment variables](env_vars.md) to specify the optimization level,
dependency locations, and more.
!alert-end!
