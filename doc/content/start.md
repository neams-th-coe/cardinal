# Getting Started

!alert! note title=tldr

On *CPU systems*, all that you need to compile Cardinal is:

```
cd $HOME
git clone https://github.com/neams-th-coe/cardinal.git
cd cardinal
./scripts/get-dependencies.sh
./contrib/moose/scripts/update_and_rebuild_petsc.sh
./contrib/moose/scripts/updatea_and_rebuild_libmesh.sh
export NEKRS_HOME=$HOME/cardinal/install
make -j8
```

If the above produces a `cardinal-opt` Cardinal executable, you can
jump straight to [#running]. If you are on a GPU system, want to customize the
build, or were not successful with the above, please consult the detailed instructions
that follow.
!alert-end!

## Access

To get access to Cardinal, simply clone the repository and
`cd` into the repository.

```
git clone https://github.com/neams-th-coe/cardinal.git
cd cardinal
```

## Prerequisites

The basic prerequisites for building Cardinal are summarized in [prereq_table].

!alert! tip title=How do I know if I have these dependencies?
Most systems will already have these available.
To figure out if you have these dependencies, check out
[our prerequisite guide](prereqs.md).
!alert-end!

!table id=prereq_table caption=Summary of prerequisites needed for Cardinal.
|    | Building with NekRS | Building with OpenMC | Both |
| :- | :- | :- | :- |
| CMake | $\checkmark$ | $\checkmark$ | $\checkmark$ |
| GNU fortran compilers | $\checkmark$ | &nbsp; | $\checkmark$  |
| HDF5 | &nbsp; | $\checkmark$ | $\checkmark$ |
| MPI | $\checkmark$ | $\checkmark$ | $\checkmark$ |

Then, decide whether you want *both* NekRS and OpenMC, just one,
or neither. Both are enabled by default, but you can
build Cardinal with only the dependencies that you want.
If you do *not*
want to build the NekRS-part of Cardinal, set the following environment variable:

```
export ENABLE_NEK=false
```

Likewise, if you do *not* want to build the OpenMC-part of Cardinal,
set the following environment variable:

```
export ENABLE_OPENMC=false
```

## Building
  id=build

#### Fetch Dependencies
  id=fetch

Cardinal has MOOSE, OpenMC, and NekRS as dependencies. However, you do not need to separately
build/compile *any* of these dependencies - Cardinal's Makefile handles all steps
automatically. To fetch the MOOSE, OpenMC, and NekRS dependencies, run:

```
./scripts/get-dependencies.sh
```

!alert! note title=Optional dependencies

Cardinal supports *optional* coupling to the following codes:

- *SAM*, a tool for systems analysis of advanced non-light water reactors
  safety analysis. Follow [these instructions](sam_instructions.md) to obtain the required dependencies for adding the
  SAM submodule.
- *Sockeye*, a tool for modeling of heat pipe systems. Follow [these instructions](sockeye_instructions.md) to obtain the required dependencies for adding the
  Sockeye submodule.
!alert-end!

#### Set Environment Variables
  id=env

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
For even further control of the build, you may want to set other
[optional environment variables](env_vars.md) to specify the optimization level,
dependency locations, and more.
!alert-end!

#### Set the OCCA Backend
  id=occa

NekRS uses [OCCA](https://libocca.org/#/) to provide an API for device programming. Available
backends include CPU (i.e. [!ac](MPI) parallelism), CUDA, HIP, OpenCL, and OpenMP.
There are several different ways that you can set the backend; in order of *decreasing* priority,

- Pass via the command line, like

  ```
  cardinal-opt -i nek.i --nekrs-backend=CPU
  ```
- Set in the `[OCCA]` block of the NekRS `.par` file to control the backend for a specific model, like

  ```
  [OCCA]
    backend = CPU
  ```
- Set the `NEKRS_OCCA_MODE_DEFAULT` environment variable to one of `CPU`, `CUDA`, `HIP`, `OPENCL`, or
  `OPENMP` to control the backend for all models, like

  ```
  export NEKRS_OCCA_MODE_DEFAULT=CPU
  ```

!alert! note title=Compiling for GPU?
If you plan to use a GPU backend, you will also need to
set the correct threading API in the `Makefile` by setting
the values of the `OCCA_CUDA_ENABLED`, `OCCA_HIP_ENABLED`, or `OCCA_OPENCL_ENABLED` variables to 1,
respectively.
!alert-end!

#### Build PETSc and libMesh
  id=petsc_libmesh

If you are using the PETSc and libMesh that ship with MOOSE's conda environment,
skip directly to [#compiling]. Otherwise,
you must now build PETSc and libMesh:

```
./contrib/moose/scripts/update_and_rebuild_petsc.sh
./contrib/moose/scripts/update_and_rebuild_libmesh.sh
```

To troubleshoot the PETSc or libMesh install, please consult our [PETSc and libMesh troubleshooting](petsc_libmesh.md) page. If you want to check the PETSc install, you can
[run the PETSc tests](petsc.md).

!alert tip
Building libMesh can be time consuming. You only need to build libMesh
if the libMesh hash used by MOOSE has been updated or this is the first time you
are building Cardinal.
On systems with multiple processors, you can set the environment
variables `JOBS`, `LIBMESH_JOBS`, and/or `MOOSE_JOBS` to be the number
of processes to use in a parallel `make` to build libMesh.  You can
also save time by restricting the
`METHOD` option.

#### Compile Cardinal
  id=compiling

Finally, run `make` in the top-level directory,

```
make -j8
```

which will compile Cardinal in parallel with 8 processes.
This will create the executable `cardinal-<mode>` in the
top-level directory. `<mode>` is the optimization level used to compile MOOSE
set with the `METHOD` environment variable.

!alert! tip
If at any point during the build process, you change your environment, then
you may need to start the build from scratch to be sure that the same
HDF5/compilers/etc. are used to build the entire set of dependencies. When
you restart the build, be sure to clear the `cardinal/build` and `cardinal/install`
directories so that the appropriate CMake configuration files will be recreated
with the newest environment settings.

```
rm -rf cardinal/install cardinal/build
```
!alert-end!

## Running
  id=running

The command to run Cardinal with `<n>` MPI ranks and `<s>` OpenMP threads is:

```
mpiexec -np <n> cardinal-opt -i input.i --n-threads=<s>
```

This command assumes that `cardinal-opt` is located on your `PATH`; otherwise,
you need to provide the full path to `cardinal-opt` in the above command.
Note that while MOOSE and OpenMC use hybrid parallelism with both MPI and OpenMP,
NekRS does not use shared memory parallelism.

!alert! tip title=Command line options
Cardinal supports all of MOOSE's command line parameters, as well as a few Cardinal-specific
command line options. For a full list:

```
cardinal-opt --help
```
!alert-end!

!alert! note title=Running with OpenMC?

If you are using OpenMC, you may require a few other dependencies either at runtime or when
setting up models.

- Follow [these instructions](cross_sections.md) to obtain cross section data.
- You may also *optionally* use OpenMC's Python [!ac](API) to build models. To use
  this [!ac](API), follow
  [these instructions](https://docs.openmc.org/en/stable/usersguide/install.html#installing-python-api).

!alert-end!

!alert! note title=Running with NekRS?

If you are using NekRS, you will require a few other dependencies when setting up models.
Follow [these instructions](nek_tools.md) to obtain binary executables to use for common NekRS-related operations, such as:

- Converting between an Exodus mesh and NekRS's custom `.re2` mesh format
- Generating metadata files for visualizing NekRS's custom field fiel output in Paraview

!alert-end!

## Checking the Install

If you would like to check that Cardinal was built correctly and that you
have all the basic requirements in place, you can try running a few input files.

1. If you are using OpenMC, try:

```
cd tutorials/lwr_solid
mpiexec -np 2 ../../cardinal-opt -i solid.i --n-threads=2
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

For developers, you will also find it useful to be able to run Cardinal's
test suite, which consists of unit and regression tests.
You can run Cardinal's test suite with the following:

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

If you run into issues running the test suite, please check out our
[run_tests troubleshooting page](run_tests_troubleshoot.md).
