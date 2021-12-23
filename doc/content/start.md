# Getting Started

This page describes how to get started using Cardinal. For additional help,
please [contact a developer](contact.md). We also have private Cardinal-specific
Slack channels on the MOOSE Developers Slack workspace
(`moosedevelopers.slack.com`) and on the NekRS Developers Slack workspace
(`nekrsdev-team.slack.com`). Feel free to reach out to a developer to be
added to these channels.

## Access

To get access to Cardinal, simply clone the repository and
`cd` into the repository (all further instructions on this page assume you
are located in the root directory of the Cardinal repository).

```
$ git clone https://github.com/neams-th-coe/cardinal.git
$ cd cardinal
```

## Building
  id=build

This section describes how to build Cardinal. Please follow each task in the order it is
presented:

1. [#fetch]
2. [#env]
3. [#occa]
4. [#petsc_libmesh]
5. [#compiling]

!alert note
Cardinal can be built with or without [MOOSE's conda environment](https://mooseframework.inl.gov/getting_started/installation/conda.html).
Any differences in the build process that depend on whether you are using the conda environment
are noted where applicable.

#### Fetch Dependencies
  id=fetch

Cardinal has MOOSE, OpenMC, and NekRS as dependencies. However, you do not need to separately
build/compile *any* of these dependencies - Cardinal's Makefile handles all steps
automatically. To fetch the MOOSE, OpenMC, and NekRS dependencies, run:

```
$ ./scripts/get-dependencies.sh
```

Your system will also require an [!ac](MPI) wrapper, the [HDF5 library](https://www.hdfgroup.org/solutions/hdf5/)
for reading OpenMC's cross sections and writing OpenMC output files,
and [CMake](https://cmake.org/) for compilation. If you use MOOSE's conda environment,
these dependencies will already be available to you. Note that at
this time, NekRS only supports
the GNU compilers.

Cardinal supports *optional* coupling to the following codes:

- *SAM*, a tool for systems analysis of advanced non-light water reactors
  safety analysis
- *Sockeye*, a tool for modeling of heat pipe systems
- *THM*, a tool for 1-D thermal-hydraulics analysis

!alert! note title=Building with an optional dependency?

- *SAM*: Follow [these instructions](sam_instructions.md) to obtain the required dependencies for adding the
  SAM submodule.
- *Sockeye*: Follow [these instructions](sockeye_instructions.md) to obtain the required dependencies for adding the
  Sockeye submodule.
- *THM*: Follow [these instructions](thm_instructions.md) to obtain the required dependencies for adding the
  THM submodule.
!alert-end!

!alert! note title=Running with OpenMC?

- Follow [these instructions](cross_sections.md) to obtain cross section data for use in OpenMC.
- You may also *optionally* use OpenMC's Python [!ac](API) to build OpenMC models. To use
  this [!ac](API), follow
  [these instructions](https://docs.openmc.org/en/stable/usersguide/install.html#installing-python-api).

!alert-end!

#### Set Environment Variables
  id=env

A number of environment variables are required or recommended when building/running Cardinal.
We recommend putting these in your `~/.bashrc`:

``` language=bash
# [REQUIRED] you must set the location of the root directory of the NekRS install;
# this will be the 'install' directory at the top level of the Cardinal repository.
# Be sure to adjust this to wherever you have cloned the Cardinal repository
export NEKRS_HOME=$HOME/cardinal/install

# [REQUIRED IF USING MOOSE CONDA ENV] you must set the location of the HYPRE
# libraries (so that a libHYPRE.so or something similar is located at
# HYPRE_DIR/lib). This is needed because NekRS uses the same HYPRE built with MOOSE
export HYPRE_DIR=$CONDA_PREFIX

# [OPTIONAL] it's a good idea to explicitly note that you are using MPI compiler wrappers
export CC=mpicc
export CXX=mpicxx
export FC=mpif90

# [OPTIONAL] if running with OpenMC, you will need cross section data at runtime;
# you will need to set this variable to point to a 'cross_sections.xml' file.
# Be sure to adjust this to wherever you have cross section data.
export OPENMC_CROSS_SECTIONS=${HOME}/cross_sections/endfb71_hdf5/cross_sections.xml
```

Additional *optional* environment variables that you may need/want to set when building
Cardinal:

- `HDF5_INCLUDE_DIR`: location of HDF5 headers
- `HDF5_LIBDIR`: location of HDF5 libraries
- `METHOD`: optimization method to use for executable; multiple executables with
   different settings will be built if specifying more than one method, such as wth
  `METHOD='opt dbg'`. Options are:

  - `opt`: optimized mode, for production runs
  - `oprof`: very slightly slower, instrumented for performance analysis with tools
     like "oprofile" or "perf"
  - `dbg`: debugging mode, much slower, with debugging symbols, optimization disabled, slow
     internal assertions enabled, and very slow libstdc++ range-checking and consistency
     checks enabled

#### Set the OCCA Backend
  id=occa

NekRS uses [OCCA](https://libocca.org/#/) to provide an API for device programming. Available
backends in NekRS include CPU (i.e. [!ac](MPI) parallelism), CUDA, HIP, OpenCL, and OpenMP.
There are several different ways that you can set the backend. We recommend setting the
`NEKRS_OCCA_MODE_DEFAULT` environment variable to one of `CPU`, `CUDA`, `HIP`, `OPENCL`, or
`OPENMP`. This will set the backend for all NekRS runs within Cardinal.

```
$ export NEKRS_OCCA_MODE_DEFAULT=CPU
```

Alternatively, if you only want to control the backend
for a particular case, you can explicitly set the backend in the NekRS input files
in the `[OCCA]` block in the `.par` file:

```
[OCCA]
  backend = CPU
```

which will override environment settings.
If you plan to use a GPU  backend, you will also need to
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
$ ./contrib/moose/scripts/update_and_rebuild_petsc.sh
$ ./contrib/moose/scripts/update_and_rebuild_libmesh.sh
```

After building PETSc, don't worry if the PETSc
tests don't pass.

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
$ make -j8
```

which will compile Cardinal in parallel with 8 processes.
This will create the executable `cardinal-<mode>` in the
top-level directory. `<mode>` is the optimization level used to compile MOOSE
set with the `METHOD` environment variable.

## Running

The command to run Cardinal with `<n>` MPI ranks and `<s>` OpenMP threads is:

```
$ mpiexec -np <n> cardinal-opt -i input.i --n-threads=<s>
```

This command assumes that `cardinal-opt` is located on your `PATH`; otherwise,
you need to provide the full path to `cardinal-opt` in the above command.
Note that while MOOSE and OpenMC use hybrid parallelism with both MPI and OpenMP,
NekRS does not use shared memory parallelism.

For the special case of running SAM as the master application, you also need to pass
`--app sam` on the command line to instruct Cardinal to build a `SamApp`.

## Testing

Cardinal uses MOOSE's CIVET system for regression testing [!cite](slaughter).
You can run Cardinal's test suite with the following:

```
$ ./run_tests -j8
```

which will run the tests in parallel with 8 processes. OpenMC's tests require
you to use the ENDF/B7-II.1 data set, which you can download following
the instructions [here](cross_sections.md).
Depending on the availability of various dependencies, some tests may be skipped.
The first time
you run the test suite, the runtime will be very long due to the just-in-time compilation of
NekRS. Subsequent runs will be much faster due to the use of cached build files.

If your Python version is old, you may encounter errors in the `run_tests` command.
To check if your installation is working, you can instead navigate to any of the
tests in the `test/tests` directory and try running that test directly (i.e. outside
the testing framework). For example, you can try to run a [!ac](CHT) for a
[!ac](SFR) pincell with:

```
$ cd test/tests/cht/sfr_pincell
$ mpiexec -np 4 cardinal-opt -i nek_master.i
```
