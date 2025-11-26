# Building Cardinal without MOOSE's conda Environment

!alert! note title=tldr

On *CPU systems*, all that you need to compile Cardinal is:

```
cd $HOME
git clone https://github.com/neams-th-coe/cardinal.git
cd cardinal
./scripts/get-dependencies.sh
./contrib/moose/scripts/update_and_rebuild_petsc.sh
./contrib/moose/scripts/update_and_rebuild_libmesh.sh
./contrib/moose/scripts/update_and_rebuild_wasp.sh
export NEKRS_HOME=$HOME/cardinal/install
make -j8
```

If the above produces a `cardinal-opt` Cardinal executable, you can
jump straight to [#running]. If you are on a GPU system, want to customize the
build, or were not successful with the above, please consult the detailed instructions
that follow.

!alert! tip title=Using pre-built dependencies
If you have some of the third-party libraries (TPLs) already installed, you can
skip building them by setting `BUILD_[TPL_NAME]=no` and `[TPL_NAME]_DIR=/path/to/tpl`.
See the "Using Pre-built Dependencies" section for more details.
!alert-end!
!alert-end!

## Access

!include access.md

## Prerequisites

!include cardinal_prereqs.md

!alert! tip title=How do I know if I have these dependencies?
Most systems will already have these available.
To figure out if you have these dependencies, check out
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
# [REQUIRED] you must set the location of the root directory of the NekRS install;
# this will be the 'install' directory at the top level of the Cardinal repository.
export NEKRS_HOME=$HOME/cardinal/install

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

#### Using Pre-built Dependencies
  id=prebuilt

By default, Cardinal will build all of its required dependencies from source. However, if you already have some of these dependencies installed, you can instruct Cardinal to use your pre-built versions. This is done by setting a `BUILD_*` variable to `no` and specifying the location of the pre-built dependency with a `*_DIR` variable.

Here is a list of the build flags and their corresponding location variables:

| Dependency   | Build Flag           | Location Variable   | Notes                                    |
|--------------|----------------------|---------------------|------------------------------------------|
| NekRS        | `BUILD_NEKRS`        | `NEKRS_DIR`         |                                          |
| OpenMC       | `BUILD_OPENMC`       | `OPENMC_DIR`        |                                          |
| DAGMC        | `BUILD_DAGMC`        | `DAGMC_DIR`         | Only relevant if `BUILD_OPENMC=yes`      |
| MOAB         | `BUILD_MOAB`         | `MOAB_DIR`          | Only relevant if `BUILD_DAGMC=yes`       |
| Embree       | `BUILD_EMBREE`       | `EMBREE_DIR`        | Only relevant if `BUILD_DOUBLEDOWN=yes`  |
| Double-Down  | `BUILD_DOUBLEDOWN`   | `DOUBLEDOWN_DIR`    | Only relevant if `BUILD_DAGMC=yes`       |

For example, to use a pre-built version of OpenMC, you would set the following environment variables:

``` language=bash
export BUILD_OPENMC=no
export OPENMC_DIR=/path/to/your/openmc
```

#### Set OCCA Backend

!include occa.md

#### Build PETSc and libMesh
  id=petsc_libmesh

You must now build PETSc, libMesh, and WASP:

```
./contrib/moose/scripts/update_and_rebuild_petsc.sh
./contrib/moose/scripts/update_and_rebuild_libmesh.sh
./contrib/moose/scripts/update_and_rebuild_wasp.sh
```

To troubleshoot the PETSc or libMesh install, please consult our [PETSc and libMesh troubleshooting](petsc_libmesh.md) page. If you want to check the PETSc install, you can
[run the PETSc tests](petsc.md).

!alert tip
Building libMesh can be time consuming. You only need to build libMesh
if the libMesh hash used by MOOSE has been updated or this is the first time you
are building Cardinal.
On systems with multiple processors, set the environment
variables `JOBS`, `LIBMESH_JOBS`, and `MOOSE_JOBS` to be the number
of processes to use in a parallel `make` to build libMesh. Please set all three
of these variables, as various parts of MOOSE's dependencies use different variables.

#### Compile Cardinal
  id=compiling

!include compiling.md

## Running
  id=running

!include running.md

## Checking the Install

!include checking_install.md
