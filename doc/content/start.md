# Getting Started

This page describes how to get started using Cardinal. For additional help,
please post to our [discussions page](https://github.com/neams-th-coe/cardinal/discussions).
We also have Cardinal-specific
Slack channels on the MOOSE Developers Slack workspace
(`moosedevelopers.slack.com`) and on the NekRS Developers Slack workspace
(`nekrsdev-team.slack.com`). Please reach out to a [developer](contact.md) to be
added to these channels.

## Access

Cardinal is openly-available on GitHub; to access, clone the repository.
Then, `cd` into the top-level of the repository (all further instructions on this
page assume you are located in the root directory of Cardinal).

```
git clone https://github.com/neams-th-coe/cardinal.git
cd cardinal
```

## Prerequisites

Before building, first decide whether you want *both* NekRS and OpenMC, just one,
or neither. You can build Cardinal with only the dependencies that you want (which reduces
the software stack prerequisites). If you want to build with both NekRS and OpenMC,
skip to [#build] - both dependencies are enabled by default.

If you do *not*
want to build the NekRS-part of Cardinal, set the following environment variable:

```
export ENABLE_NEK=false
```

Likewise, if you do *not* want to build the OpenMC-part of Cardinal, set:

```
export ENABLE_OPENMC=false
```

You can also edit both of these environment variables in Cardinal's `Makefile`.
Cardinal's prerequisites are summarized in [prereq_table].

!table id=prereq_table caption=Summary of prerequisites needed for Cardinal.
|    | Building with NekRS | Building with OpenMC | Both |
| :- | :- | :- | :- |
| CMake | $\checkmark$ | $\checkmark$ | $\checkmark$ |
| GNU compilers | $\checkmark$ | &nbsp; | $\checkmark$  |
| HDF5 | &nbsp; | $\checkmark$ | $\checkmark$ |
| MPI | $\checkmark$ | $\checkmark$ | $\checkmark$ |

!alert note
If you use [MOOSE's conda environment](https://mooseframework.inl.gov/getting_started/installation/conda.html)
 on Linux, all of these requirements will automatically
be available to you. If you are on MacOS, MOOSE's conda environment only contains the
clang compiler, so you will separately need to obtain the GNU compilers.

## Building
  id=build

This section describes how to build Cardinal. Please follow each task in the order it is
presented:

1. [#fetch]
2. [#env]
3. [#occa]
4. [#petsc_libmesh]
5. [#compiling]

#### Fetch Dependencies
  id=fetch

Cardinal has MOOSE, OpenMC, and NekRS as dependencies. However, you do not need to separately
build/compile *any* of these dependencies - Cardinal's Makefile handles all steps
automatically. To fetch the MOOSE, OpenMC, and NekRS dependencies, run:

```
./scripts/get-dependencies.sh
```

!alert! note title=Optional submodules

Cardinal can be coupled to *any* MOOSE application. For convenience, we have a few commonly-used
MOOSE applications available as optional submodules:

- *SAM*, for systems analysis of advanced non-light water reactors. Follow [these instructions](sam_instructions.md) to get SAM.
- *Sockeye*, for heat pipe systems. Follow [these instructions](sockeye_instructions.md) to get Sockeye.
!alert-end!

!alert! note title=Running with OpenMC?

- Follow [these instructions](cross_sections.md) to obtain cross section data.
- You may also *optionally* use OpenMC's Python [!ac](API) to build OpenMC models. To use
  this [!ac](API), follow
  [these instructions](https://docs.openmc.org/en/stable/usersguide/install.html#installing-python-api).

!alert-end!

!alert! note title=Running with NekRS?

Follow [these instructions](nek_tools.md) to obtain binary executables to use for common NekRS-related operations, such as:

- Converting between an Exodus mesh and NekRS's custom `.re2` mesh format
- Generating metadata files for visualizing NekRS's custom field fiel output in Paraview

!alert-end!

#### Set Environment Variables
  id=env

A number of environment variables are required or recommended when building/running Cardinal.

``` language=bash
# [REQUIRED] you must set the location of the root directory of the NekRS install;
# this will be the 'install' directory at the top level of the Cardinal repository.
# Be sure to adjust this to wherever you have cloned the Cardinal repository
export NEKRS_HOME=$HOME/cardinal/install

# [OPTIONAL] it's a good idea to explicitly note that you are using MPI compiler wrappers
export CC=mpicc
export CXX=mpicxx
export FC=mpif90

# [OPTIONAL] if running with OpenMC, you will need cross section data at runtime;
# you will need to set this variable to point to a 'cross_sections.xml' file.
# Be sure to adjust this to wherever you have cross section data.
export OPENMC_CROSS_SECTIONS=${HOME}/cross_sections/endfb71_hdf5/cross_sections.xml
```

Additional *optional* environment variables that you may want to set:

- `HDF5_ROOT`: top-level directory containing HDF5 (this directory should contain
   an `include` and a `lib` directory). If not set, this will default to the HDF5 downloaded
   by PETSc (which is placed into `$PETSC_DIR/$PETSC_ARCH`).

- `METHODS`: optimization method(s) to use for building libMesh.
   Multiple libMesh libraries with different settings will be built if specifying more than one method, such as wth
  `METHODS='opt dbg'`. Options are:

  - `opt`: optimized mode, for production runs
  - `oprof`: very slightly slower, instrumented for performance analysis with tools
     like "oprofile" or "perf"
  - `dbg`: debugging mode, much slower, with debugging symbols, optimization disabled, slow
     internal assertions enabled, and very slow libstdc++ range-checking and consistency
     checks enabled

- `METHOD`: optimization method to use for Cardinal. Only one Cardinal executable can be built at once,
  such as via
  `METHOD='opt'`. If you want to build multiple Cardinal executables with
  different optimization methods, run `make` multiple times with different
  `METHOD` values.

#### Set the OCCA Backend
  id=occa

NekRS uses [OCCA](https://libocca.org/#/) to provide an API for device programming. Available
backends in NekRS include CPU (i.e. [!ac](MPI) parallelism), CUDA, HIP, OpenCL, and OpenMP.
There are several different ways that you can set the backend. We recommend setting the
`NEKRS_OCCA_MODE_DEFAULT` environment variable to one of `CPU`, `CUDA`, `HIP`, `OPENCL`, or
`OPENMP`.

```
export NEKRS_OCCA_MODE_DEFAULT=CPU
```

If you only want to control the backend
for a particular case, you can explicitly set the backend in the `[OCCA]` block of NekRS's
`.par` file:

```
[OCCA]
  backend = CPU
```

which will override environment settings.
If you plan to use a GPU backend, you will also need to
set the correct threading API in the `Makefile` by setting
the values of the `OCCA_CUDA_ENABLED`, `OCCA_HIP_ENABLED`, or `OCCA_OPENCL_ENABLED` variables,
respectively.

#### Build PETSc and libMesh
  id=petsc_libmesh

!alert note
If you are using the PETSc and libMesh that ship with MOOSE's conda environment,
set `PETSC_DIR` and `PETSC_ARCH` to the PETSc install directory and architecture name
and `LIBMESH_DIR` to the libMesh install directory.
Then, skip directly to [#compiling].

If not using MOOSE's conda environment, you must now build PETSc and libMesh.
To build PETSc and libMesh, run:

```
./contrib/moose/scripts/update_and_rebuild_petsc.sh
./contrib/moose/scripts/update_and_rebuild_libmesh.sh
```

After building PETSc, if you want to test the installation you will need to `cd`
into the PETSc directory before running the on-screen directions that print
when PETSc finishes, i.e. like:

```
cd contrib/moose/petsc
make PETSC_DIR=$HOME/cardinal/contrib/moose/scripts/../petsc PETSC_ARCH=arch-moose check
```

!alert tip
Building PETSc and libMesh can be time consuming. You only need to build these
if the PETSc or libMesh hashes used by MOOSE has been updated or this is the first time you
are building Cardinal.
On systems with multiple processors, you can set the environment
variables `JOBS`, `LIBMESH_JOBS`, and/or `MOOSE_JOBS` to be the number
of processes to use in a parallel `make` to build libMesh.

#### Compile Cardinal
  id=compiling

Finally, run `make` in the top-level directory,

```
make -j8
```

which will compile Cardinal in parallel with 8 processes.
This will create the executable `cardinal-<mode>` in the
top-level directory. `<mode>` is the optimization level used to compile MOOSE
set with the `METHOD` variable.

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

To run Cardinal with `<n>` MPI ranks and `<s>` OpenMP threads is:

```
mpiexec -np <n> cardinal-opt -i input.i --n-threads=<s>
```

This command assumes that `cardinal-opt` is located on your `PATH`; otherwise,
you need to provide the full path to `cardinal-opt`.
Note that while MOOSE and OpenMC use hybrid parallelism with both MPI and OpenMP,
NekRS does not use shared memory parallelism.

Finally, for the special case of running SAM as the master application, you also need to pass
`--app SamApp` on the command line to instruct Cardinal to build a `SamApp`.

## Checking the Install

If you would like to check that Cardinal was built correctly and that you
have all the basic requirements in place, you can try running a few input files.

1. If you are using OpenMC, try:

```
cd tutorials/lwr_solid
mpiexec -np 2 ../../cardinal-opt -i solid.i --n-threads=2
```

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
