# Building Cardinal with MOOSE's conda Environment

!alert! warning title=Incompatible with NekRS
MOOSE's conda environment is currently incompatible with Cardinal's
NekRS wrapping. MOOSE's environment exports some header files that point
to HYPRE, but NekRS needs to use its own special version of HYPRE. The MOOSE
team is not aware of any way to exclude header files attached to MOOSE's
conda compiler wrappers.

You MUST set `export ENABLE_NEK=false` if you want to use the conda environment.
Otherwise, if you do want NekRS features, you need to instead follow the
[instructions for building without the conda environment](without_conda.md).
We hope to relax this restriction in the future - if this
is impacting your work, please let us know so that we can increase its priority.
!alert-end!

If you're only interested in OpenMC, then you're in the right place -
please follow the instructions on this page. Note that you should NOT be building
OpenMC separately, or using its conda environment (i.e., do
`conda deactivate openmc` if you were using OpenMC's conda).

!alert! note title=tldr

All that you need to compile Cardinal is:

```
conda activate moose
cd $HOME
git clone https://github.com/neams-th-coe/cardinal.git
cd cardinal
./scripts/get-dependencies.sh
export ENABLE_NEK=false
export HDF5_ROOT=$CONDA_PREFIX
export LD_LIBRARY_PATH=$CONDA_PREFIX/lib:$LD_LIBRARY_PATH
make -j8
```

If the above produces a `cardinal-opt` Cardinal executable, you can
jump straight to [#running]. If you want to customize the
build or were not successful with the above, please consult the detailed instructions
that follow.
!alert-end!

## Access

!include access.md

To get MOOSE's conda environment, follow the instructions [here](https://mooseframework.inl.gov/getting_started/installation/index.html).

## Prerequisites

!include cardinal_prereqs.md

!alert! tip title=How do I know if I have these dependencies?
You will already have all of these if using MOOSE's conda environment.
But if you want to learn more, check out
[our prerequisite guide](prereqs.md).
!alert-end!

!include options.md

## Building
  id=build

#### Fetch Dependencies
  id=fetch

!include dependencies.md

#### Set Environment Variables
  id=env

A number of environment variables are required or recommended when building/running Cardinal.
Put these in your `~/.bashrc` (don't forget to `source ~/.bashrc`!):

``` language=bash
# [REQUIRED IF USING MOOSE CONDA ENV for HDF5] you must set the location of the
# root HDF5 directory provided by MOOSE for OpenMC to find
export HDF5_ROOT=$CONDA_PREFIX

# [REQUIRED ON SOME SYSTEMS] for some systems, libraries won't be linked properly unless
# you explicitly point this variable. We're working on a more elegant fix.
export LD_LIBRARY_PATH=$CONDA_PREFIX/lib:$LD_LIBRARY_PATH

# [OPTIONAL] it's a good idea to explicitly note that you are using MPI compiler wrappers
export CC=mpicc
export CXX=mpicxx
export FC=mpif90

# [OPTIONAL] if running with OpenMC, you will need cross section data at runtime;
# you will need to set this variable to point to a 'cross_sections.xml' file.
export OPENMC_CROSS_SECTIONS=${HOME}/cross_sections/endfb-vii.1-hdf5/cross_sections.xml
```

!alert! tip title=Additional environment variables
For even further control, you can set other
[optional environment variables](env_vars.md) to specify the optimization level,
dependency locations, and more.
!alert-end!

#### Compile Cardinal
  id=compiling

!include compiling.md

## Running
  id=running

!include running.md

## Checking the Install

!include checking_install.md
