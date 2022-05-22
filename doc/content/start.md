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

## Prerequisites

Before building, first decide whether you want *both* NekRS and OpenMC, just one,
or neither. You can build Cardinal with only the dependencies that you want (which reduces
the software stack prerequisites). If you do *not*
want to build the NekRS-part of Cardinal, set the following environment variable:

```
$ export ENABLE_NEK=false
```

Likewise, if you do *not* want to build the OpenMC-part of Cardinal,
set the following environment variable:

```
$ export ENABLE_OPENMC=false
```

You can also edit both of these variables in Cardinal's `Makefile`. Then, the prerequisites
for building Cardinal are summarized in [prereq_table].

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
$ ./scripts/get-dependencies.sh
```


!alert! note title=Optional dependencies

Cardinal supports *optional* coupling to the following codes:

- *SAM*, a tool for systems analysis of advanced non-light water reactors
  safety analysis. Follow [these instructions](sam_instructions.md) to obtain the required dependencies for adding the
  SAM submodule.
- *Sockeye*, a tool for modeling of heat pipe systems. Follow [these instructions](sockeye_instructions.md) to obtain the required dependencies for adding the
  Sockeye submodule.
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

- `HDF5_ROOT`: top-level directory containing HDF5 (this directory should contain
   an `include` and a `lib` directory). If not set, this will default to the HDF5 downloaded
   by PETSc and placed into `$PETSC_DIR/$PETSC_ARCH`.
- `METHODS`: optimization method(s) to use for building Cardinal's libMesh dependency.
   Multiple libMesh libraries with different settings will be built if specifying more than one method, such as wth
  `METHODS='opt dbg'`. Options are:

  - `opt`: optimized mode, for production runs
  - `oprof`: very slightly slower, instrumented for performance analysis with tools
     like "oprofile" or "perf"
  - `dbg`: debugging mode, much slower, with debugging symbols, optimization disabled, slow
     internal assertions enabled, and very slow libstdc++ range-checking and consistency
     checks enabled

- `METHOD`: optimization method to use when building Cardinal. MOOSE currently only supports
  building one Cardinal executable at a time, such as via
  `METHOD='opt'`. If you want to build multiple Cardinal executables with
  different optimization methods, you will need to run `make` multiple times with different
  `METHOD` values.

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

After building PETSc, if you want to test the installation you will need to `cd`
into the PETSc directory before running the on-screen directions that print
when PETSc finishes, i.e. like:

```
$ cd contrib/moose/petsc
$ make PETSC_DIR=$HOME/cardinal/contrib/moose/scripts/../petsc PETSC_ARCH=arch-moose check
```

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

!alert! tip
If at any point during the build process, you change your environment, then
you may need to start the build from scratch to be sure that the same
HDF5/compilers/etc. are used to build the entire set of dependencies. When
you restart the build, be sure to clear the `cardinal/build` and `cardinal/install`
directories so that the appropriate CMake configuration files will be recreated
with the newest environment settings.

```
$ rm -rf cardinal/install cardinal/build
```
!alert-end!

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
`--app SamApp` on the command line to instruct Cardinal to build a `SamApp`.

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

!alert! note
Do you get an error like the following when running the test suite?

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
Do you get an error like the following when running the test suite?

```
Error! Could not find 'libmesh-config' in any of the usual libmesh's locations!
```

This means that you need to explicitly set `LIBMESH_DIR` to point
to where you have `moose/libmesh`. If you're using MOOSE's conda environment, this
means setting `LIBMESH_DIR` to `$CONDA_PREFIX/libmesh`. If not using the conda
environment, set to `cardinal/contrib/moose/libmesh`.

!alert-end!

You can also try running the various test input files directly (i.e. outside
the testing framework). For example, you can try to run a [!ac](CHT) for a
[!ac](SFR) pincell with:

```
$ cd test/tests/cht/sfr_pincell
$ mpiexec -np 4 cardinal-opt -i nek_master.i
```
