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

#### Fetch Dependencies
  id=fetch

Cardinal contains MOOSE, OpenMC, and NekRS as dependencies. However, you do not need to separately
build/compile *any* of these dependencies - Cardinal's Makefile handles all steps
automatically. Cardinal uses submodules
for its dependencies; to fetch the MOOSE, OpenMC, and NekRS dependencies, run:

```
$ ./scripts/get-dependencies.sh
```

Your system will also require an [!ac](MPI) wrapper, the [HDF5 library](https://www.hdfgroup.org/solutions/hdf5/)
for reading OpenMC's cross sections and writing OpenMC output files,
and [CMake](https://cmake.org/) for compilation. At this time, NekRS only supports
the GNU compilers.

Cardinal supports *optional* coupling to the following codes:

- SAM, a systems analysis tool for advanced non-light water reactor
  safety analysis
- Sockeye, a tool for modeling of heat pipe systems
- THM, a tool for 1-D thermal-hydraulics analysis

!alert! note title=Building with an optional dependency?

- *SAM*: Follow [these instructions](sam_instructions.md) to obtain the required dependencies for adding the
  SAM submodule to Cardinal.
- *Sockeye*: Follow [these instructions](sockeye_instructions.md) to obtain the required dependencies for adding the
  Sockeye submodule to Cardinal.
- *THM*: Follow [these instructions](thm_instructions.md) to obtain the required dependencies for adding the
  THM submodule to Cardinal.
!alert-end!

!alert! note title=Running with OpenMC?

- Follow [these instructions](cross_sections.md) to obtain cross section data for use in OpenMC.
- You may also *optionally* use OpenMC's Python [!ac](API) to build OpenMC models. To use
  this [!ac](API), follow
  [these instructions](https://docs.openmc.org/en/stable/usersguide/install.html#installing-python-api).

!alert-end!

#### Set Environment Variables
  id=env

A number of environment variables are required or recommended when building and running Cardinal.
We recommend putting these into your `~/.bashrc`:

``` language=bash
# [REQUIRED] you must set the location of the root directory of the NekRS install;
# this will be the 'install' directory at the top level of the Cardinal repository.
# Be sure to adjust this to wherever you have cloned the Cardinal repository
export NEKRS_HOME = $HOME/cardinal/install

# [OPTIONAL] it's a good idea to explicitly note that you are using MPI compiler wrappers
export CC=mpicc
export CXX=mpicxx
export FC=mpif90

# [OPTIONAL] if running with OpenMC, you will need cross section data at runtime;
# you will need to set this variable to point to a 'cross_sections.xml' file.
# Be sure to adjust this to wherever you have cross section data.
export OPENMC_CROSS_SECTIONS=${HOME}/cross_sections/endfb71_hdf5/cross_sections.xml
```

Additional *optional* environment variables that you may need to set in order
to build Cardinal:

- `HDF5_INCLUDE_DIR`: location of HDF5 headers
- `HDF5_LIBDIR`: location of HDF5 libraries

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
for your simulation. To do set, set the following in NekRS's `.par` file in the `[OCCA]` block:

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

After obtaining all necessary submodules and setting the relevant
environment variables and Makefile settings, the first software that must be built is the PETSc numerics library:

```
$ ./contrib/moose/scripts/update_and_rebuild_petsc.sh
```

If a MOOSE-compatible version of PETSc is already installed, it can
be used instead of the contrib version, after setting the environment
variables `PETSC_DIR` and `PETSC_ARCH` to the PETSc install directory
and architecture name. After building PETSc, don't worry if the PETSc
tests don't pass - please complete the entire set of instructions here.

Second, build the libMesh finite element library:

```
$ ./contrib/moose/scripts/update_and_rebuild_libmesh.sh
```

If a MOOSE-compatible version of libMesh is already installed, it can
be used instead of the contrib version, after setting the environment
variable `LIBMESH_DIR` to the libMesh install directory.  Take care if
attempting this: MOOSE is very quick to adopt newly-added libMesh
APIs, and is often only compatible with a recent libMesh git HEAD,
not with even the most recent official libMesh release version.

Building libMesh can be quite time consuming. You only need to perform the above step
if the MOOSE submodule has been updated or this is the first time you are building Cardinal.
On systems with multiple processors, you can first set the environment
variables `JOBS`, `LIBMESH_JOBS`, and/or `MOOSE_JOBS` to be the number
of processes to use in a parallel `make` to build libMesh.  You can
also save time by restricting (or add flexibility by expanding) the
`METHOD` options available later (see [#compiling]) by setting the
environment variable `METHODS`, e.g. `oprof dbg` would only build
those two modes, not all three default builds.

#### Compiling Cardinal
  id=compiling

Finally, run `make` in the top-level directory,

```
$ make -j8
```

which will compile Cardinal in parallel with 8 processes.
This will create the executable `cardinal-<mode>` in the
top-level directory. `<mode>` is the optimization level used to compile MOOSE. You can control
this mode with the `METHOD` environment variable, which by default can
be set to any combination of `opt` (optimized mode, for production
runs), `oprof` (very slightly slower, instrumented for performance
analysis with tools like "oprofile" or "perf"), and `dbg` (debugging
mode, *much* slower, with debugging symbols, optimization disabled,
slow internal assertions enabled, and very slow libstdc++
range-checking and consistency checks enabled).  If a non-default
`METHODS` variable was used above, you may also have `devel` (slower
than oprof, much faster than dbg) or `prof` (slightly slower than
oprof, instrumented for tools like "gprof") available.

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
