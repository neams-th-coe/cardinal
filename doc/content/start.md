# Getting Started

This page describes how to get started using Cardinal. Please complete all steps
of these instructions before attempting any of the [tutorials](tutorials/index.md).

## Access

To get access to Cardinal, please [contact](contact.md) a member of
the Cardinal development team, with a description of your intended use case.
After you have been granted access, simply clone the Cardinal repository:

```
$ git clone https://github.com/neams-th-coe/cardinal.git
```

## Building

This section describes how to build Cardinal. Please follow each task in the order it is
presented:

1. [#fetch]
2. [#env]
3. [#occa]
4. [#petsc_libmesh]
5. [#compiling]

#### Fetch Dependencies
  id=fetch

Cardinal contains a number of dependencies. However, you do not need to separately
build/compile *any* of these dependencies - Cardinal's Makefile handles all compilation steps
automatically.

Cardinal uses [git submodules](https://git-scm.com/book/en/v2/Git-Tools-Submodules)
for all dependencies. [table1] summarizes all the submodule dependencies in Cardinal;
some of these dependencies are open source, while others require you to separately
apply for code access from the organization that owns that application. For instance,
if you want to use SAM (an optional, closed-source dependency) with Cardinal, you will
first need to apply for SAM access with ANL. [table1] shows, for each dependency, whether
that dependency is open source, required to build Cardinal, and where you might request
access to that dependency if it is not open source. Finally, the last column shows the
git command used to fetch that submodule dependency.

!table id=table1 caption=Summary of all submodule dependencies in Cardinal
| Dependency | Required? | Open Source? | git Command |
| :- | :- | :- | :- |
| MOOSE | yes | yes | `git submodule update --init contrib/moose` |
| OpenMC | yes | yes | `git submodule update --init --recursive contrib/openmc` |
| NekRS | yes | yes | `git submodule udpate --init contrib/nekRS` |
| SAM | no | no | `git submodule update --init contrib/SAM` |

To get access to the optional SAM dependency, please contact the
[SAM development team](https://www.anl.gov/nse/system-analysis-module).

To build Cardinal with only the open-source, freely-available dependencies,
we next need to fetch the open source submodules.

```
$ git submodule update --init contrib/moose
$ git submodule update --init --recursive contrib/openmc
$ git submodule udpate --init contrib/moose
```

If you also want to build with one of the optional dependencies, then you will
also need to fetch that dependency. For instance, to get the SAM dependency, use:

```
$ git submodule update --init contrib/SAM
```

You will then be prompted to enter credentials to access ANL's gitlab site.

#### Set Environment Variables
  id=env

Next, you must set the environment variable `NEKRS_HOME` to be the location of the
NekRS root directory so that all needed include files are on your path.
This will be the `install/` directory under the top-level Cardinal directory:

```
$ export NEKRS_HOME=$(realpath install/)
```

To run any problems using OpenMC, you will need to specify a path to
cross section data. Cross section data is not needed to simply compile Cardinal -
only to run an OpenMC case.
A detailed description of how to specify cross section
data can be found in the [OpenMC documentation](https://docs.openmc.org/en/stable/usersguide/cross_sections.html).
Any cross section data set may be used when using OpenMC within Cardinal; however,
the test suite assumes you are using the ENDF/B7-II.1 data set, which has data for temperatures
between 250 K and 2500 K. Download this library from [here](https://openmc.org/official-data-libraries/)
and then set the `OPENMC_CROSS_SECTIONS` environment variable to the location of the
`cross_sections.xml` file. For example, if you placed the cross-section data in
`${HOME}/cross_sections`, this environment variable would be set to:

```
$ export OPENMC_CROSS_SECTIONS=${HOME}/cross_sections/endfb71_hdf5/cross_sections.xml
```

If you don't already explicitly state in your `~/.bashrc` that you are using the MPI
compilers wrappers, it's also a good idea to do so:

```
export CC=mpicc
export CXX=mpicxx
export FC=mpif90
```

Additional optional environment variables that you may need to set in order
to build Cardinal:

- `HDF5_INCLUDE_DIR`: location of HDF5 headers. If `HDF5_ROOT` is specified, this
  defaults to `$HDF5_ROOT/include`; otherwise, this defaults to `/usr/include`
- `HDF5_LIBDIR`: location of HDF5 libraries. If `$HDF5_ROOT` is specified, this
  defaults to `$HDF5_ROOT/lib`; otherwise, this defaults to `/usr/lib`

#### Set the OCCA Backend
  id=occa

NekRS uses [OCCA](https://libocca.org/#/) to provide an API for device programming. Available
backends in NekRS include CPU (i.e. MPI parallelism), CUDA, HIP, OpenCL, and OpenMP. Before
building Cardinal, please set the type of backend you would like to use.

There are several different ways that you can set the backend. We recommend setting the
`NEKRS_OCCA_MODE_DEFAULT` environment variable to one of `CPU`, `CUDA`, `HIP`, `OPENCL`, or
`OPENMP`. This will set the backend for all NekRS runs within Cardinal.

```
$ export NEKRS_OCCA_MODE_DEFAULT=CPU
```

Alternatively, if you only want to control the backend
for a particular case, you can explicitly set the backend in NekRS's `.par` file in the `[OCCA]` block:

```
[OCCA]
  backend = CPU
```

which will override any other settings. Finally,
you can also set the default backend
by changing the value of the `OCCA_MODE_DEFAULT` variable in `${NEKRS_HOME}/nekrs.conf`:

```
OCCA_MODE_DEFAULT = CPU
```

`nekrs.conf` is created as part of the build process, so if you are attempting to set the
OCCA backend before actually building Cardinal, the `nekrs.conf` file won't exist. So, you would
need to first complete all build steps up through and including [#compiling] before trying
this approach (therefore, we don't recommend it).

The order of precedence for these three options
is `.par` file > `NEKRS_OCCA_MODE_DEFAULT` environment variable > `nekrs.conf`.

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

Finally, in the top-level directory, run `make`.

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

The command to run Cardinal is

```
$ mpiexec -np <n> cardinal-opt -i input.i --n-threads=<s>
```

where `n` is the number of MPI ranks and `s` is the number of OpenMP threads.
While MOOSE and OpenMC use hybrid parallelism with both MPI and OpenMP,
NekRS does not use shared memory parallelism.

For the special case of running SAM as the master application, you also need to pass
`--app sam` on the command line to instruct Cardinal to build a `SamApp`.

## Testing

Cardinal uses MOOSE's CIVET system for regression testing [!cite](slaughter).
You can run Cardinal's test suite with the following:

```
$ ./run_tests -j8
```

which will run the tests in parallel with 8 processes.
Depending on the availability of the SAM submodule, some tests may be skipped. The first time
you run the test suite, the runtime will be very long due to the just-in-time compilation of
nekRS. Subsequent runs will be much faster due to the use of cached build files.

If your Python version is old, you may encounter errors in the `run_tests` command.
To check if your installation is working, you can instead navigate to any of the
tests in the `test/tests` directory and try running that test directly (i.e. outside
the testing framework). For example, to run the test in the `test/tests/cht/sfr_pincell`
directory, run

```
$ mpiexec -np 4 cardinal-opt -i nek_master.i
```

