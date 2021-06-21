# Cardinal

Cardinal is a wrapping of the spectral element code [nekRS](https://github.com/Nek5000/nekRS) and
the Monte Carlo code [OpenMC](https://github.com/openmc-dev/openmc) as a MOOSE application.
Cardinal is intended for providing high-resolution thermal-hydraulics
and particle transport feedback to MOOSE multiphysics simulations.

## Setting the OCCA Backend

nekRS uses [OCCA](https://libocca.org/#/) to provide an API for device programming. Available
backends in nekRS include CPU (i.e. MPI parallelism), CUDA, HIP, OpenCL, and OpenMP. Before
building Cardinal, please set the type of backend you would like to use.

There are several different ways that you can set the backend. We recommend setting the
`NEKRS_OCCA_MODE_DEFAULT` environment variable to one of `CPU`, `CUDA`, `HIP`, `OPENCL`, or
`OPENMP`. This will set the backend for all nekRS runs within Cardinal.

```
$ export NEKRS_OCCA_MODE_DEFAULT=CPU
```

Equivalently, you can also set the default backend
by changing the value of the `OCCA_MODE_DEFAULT` variable in `${NEKRS_HOME}/nekrs.conf`:

```
OCCA_MODE_DEFAULT = CPU
```

`nekrs.conf` is created as part of the build process, but changing the value of this variable
will control the backend used at runtime. Alternatively, if you only want to control the backend
for a particular case, you can explicitly set the backend in nekRS's `.par` file in the `[OCCA]` block:

```
[OCCA]
  backend = CPU
```

which will override any other settings. The order of precedence for these three options
is `.par` file > `NEKRS_OCCA_MODE_DEFAULT` environment variable > `nekrs.conf`.

If you plan to use a GPU  backend, then you will also need to
set the correct threading API in the `Makefile` by setting
the values of the `OCCA_CUDA_ENABLED`, `OCCA_HIP_ENABLED`, or `OCCA_OPENCL_ENABLED` variables,
respectively.

## Building

This section describes how to compile Cardinal. Compilation of all dependencies (MOOSE, OpenMC,
nekRS, and SAM) is handled through Cardinal's Makefiles - you don't need to separately build any
dependencies. The SAM dependency is *optional* - if you do not pull the SAM dependency, then
Cardinal will still build, run, and test fine without SAM.

### Fetch the Submodules

First, fetch all the submodules containing the MOOSE, nekRS, OpenMC, and SAM dependencies. If you
will *not* be using the SAM submodule within Cardinal, you will need to individually clone
only the open-source submodules:

```
$ git submodule update --init contrib/nekRS
$ git submodule update --init contrib/moose
$ git submodule update --init --recursive contrib/openmc
```

If you are using SAM (which is an optional dependncy), you will then also need to fetch the SAM submodule:

```
$ git submodule update --init contrib/SAM
```

You will be prompted for login credentials to ANL's xgitlab site. If you do not have credentials,
you will need to contact the SAM development team to request access.

### Set the MPI Wrappers

If you don't already explicitly state in your `~/.bashrc` that you are using the MPI
compilers wrappers, it's a good idea to do so:

```
export CC=mpicc
export CXX=mpicxx
export FC=mpif90
```

### Build Petsc

After obtaining all necessary submodules, the first software that must be built is the PETSc numerics library:

```
$ ./contrib/moose/scripts/update_and_rebuild_petsc.sh
```

If a MOOSE-compatible version of PETSc is already installed, it can
be used instead of the contrib version, after setting the environment
variables `PETSC_DIR` and `PETSC_ARCH` to the PETSc install directory
and architecture name. After building PETSc, don't worry if the PETSc
tests don't pass - please complete the entire set of instructions here.

### Build libMesh

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
`METHOD` options available later (see below) by setting the
environment variable `METHODS`, e.g. `oprof dbg` would only build
those two modes, not all three default builds.

### Set Environment Variables

Next, you must set the environment variable `NEKRS_HOME` to be the location of the
nekRS root directory so that all needed include files are on your path.
This will be the `install/` directory under the top-level Cardinal directory:

```
$ export NEKRS_HOME=$(realpath install/)
```

To run any problems using OpenMC, you will need to specify a path to
cross section data. A detailed description of how to specify cross section
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

### Compile

Finally, in the top-level directory, run `make`.  This will create the executable `cardinal-<mode>` in the
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

### Optional Make Variables

* `HDF5_INCLUDE_DIR`: Specify location of HDF5 headers.  Defaults to:
  * `$HDF5_ROOT/include` if `HDF5_ROOT` is specified
  * `/usr/include` if `HDF5_ROOT` is not specified
* `HDF5_LIBDIR`: Specify location of HDF5 libraries.  Defaults to:
  * `$HDF5_ROOT/lib` if `HDF5_ROOT` is specified
  * `/usr/lib` if `HDF5_ROOT` is not specified

## Running

While most MOOSE applications contain a single "App" (like `PronghornApp` for
Pronghorn, or `BisonApp` for BISON), Cardinal contains three applications -

1. `CardinalApp`, to which are registered any generic classes in Cardinal to facilitate
   coupling.
2. `NekApp`, to which are registered any classes dealing specifically with wrapping of nekRS.
3. `OpenMCApp`, to which are registered any classes dealing specifically with wrapping of OpenMC.

In other words, you could entirely separate out the nekRS wrapping into a separate
MOOSE application named `Nek` that you create with
[Stork](https://mooseframework.inl.gov/getting_started/examples_and_tutorials/tutorial01_app_development/step01_moose_app.html)
like you do when creating any new MOOSE application. Likewise, you could entirely separate
out the OpenMC wrapping into a separate MOOSE application named `OpenMC`. This design
philosophy is used in Cardinal because the initialization steps for OpenMC and nekRS
require different function calls to routines like `nekrs::setup` and `openmc_init`
that we don't necessarily _always_ want to call if we are omitting one of the physics
from a particular simulation.

Further, if you are running SAM as the master application, Cardinal will also instantiate
a `SamApp` so that SAM classes and input file syntax can be run with the Cardinal executable.

The various command line flags you need to pass to run a Cardinal input depends on
which application is the master app (`CardinalApp`, `NekApp`, `OpenMCApp`, `SamApp`). The
`--app` flag indicates which application is to be created as the master application
for a given input file. If you omit `--app <app>`, then by default the input
file is run with `CardinalApp`. Otherwise, valid options are:

1. `--app openmc`, run the input file with `OpenMCApp`
2. `--app nek`, run the input file with `NekApp`
3. `--app sam`, run the input file with `SamApp`

Finally, if the master input file or any of the sub-application input files are to be
run with `NekApp`, you must also pass `--nekrs-setup <case>` on the command line, where
`<case>` is the nekRS caes name.

A few (non-exhaustive) examples are now provided. To run
an input file with a `CardinalApp` master application that doesn't contain nekRS as a sub-app, use

```
$ mpirun -np 4 ~/repos/cardinal/cardinal-opt -i input.i
```

To run an input file with a `CardinalApp` master application that contains nekRS as a sub-app, use

```
$ mpirun -np 4 ~/repos/cardinal/cardinal-opt -i input.i --nekrs-setup brick
```

where the nekRS input files for the case are named `brick.re2`, `brick.par`, `brick.udf`, and
`brick.oudf`. To run an input file with a `NekApp` master application, use

```
$ mpirun -np 4 ~/repos/cardinal/cardinal-opt --app nek -i nek.i --nekrs-setup brick
```

And to run an input file with an `OpenMCApp` master application that contains
nekRS as a sub-app, use

```
$ mpirun -np 4 ~/repos/cardinal/cardinal-opt --app openmc -i openmc.i --nekrs-setup brick
```

## Compiling on HPC Systems

Cardinal has been built and run on several HPC systems. To help streamline the compilation
process for other users, this section collects instructions for the modules and any
systems-specific settings for a variety of HPC systems. Because default modules and
settings change on HPC systems with time, the instructions below may become deprecated.
The date at which the settings were last tested is given for each system.

### Laboratory Computing Research Center (LCRC) [5/24/2021]

The following `~/.bashrc` allows you to compile Cardinal on LCRC, in combination
with the compilation steps described above.

```
if [ -f /etc/bashrc ]; then
        . /etc/bashrc
fi

export NEKRS_HOME=$HOME/cardinal/install

module purge
module load gcc/8.2.0-g7hppkz
module load intel-mpi/2018.4.274-ozfo327
module load intel-mkl/2018.4.274-2amycpi
module load cmake/3.14.2-gvwazz3
module load hdf5/1.8.16-mz7lmxh
module load anaconda3

export CC=mpicc
export CXX=mpicxx
export FC=mpif90
```

### Sawtooth (INL-HPC) [4/29/2021]

The following `~/.bashrc` allows you to compile Cardinal on Sawtooth, in combination
with the compilation steps described above.

```
if [ -f /etc/bashrc ]; then
        . /etc/bashrc
fi
export NEKRS_HOME=$HOME/cardinal/install
module purge
module load openmpi/4.0.5_ucx1.9
module load cmake/3.16.2-gcc-9.3.0-tza7
module load hdf5/1.12.0_ucx1.9
export CC=mpicc
export CXX=mpicxx
export FC=mpif90
```

## Testing

You can run Cardinal's test suite with the following:

```
$ ./run_tests -j8
```

Depending on the availability of the SAM submodule, some tests may be skipped. The first time
you run the test suite, the runtime will be very long due to the just-in-time compilation of
nekRS. Subsequent runs will be much faster due to the use of cached build files.

## Updating the Submodules

Cardinal uses submodules for its dependencies - MOOSE, OpenMC, nekRS, and SAM. All dependencies
except for nekRS point to the main repository remotes for each application. However, in order to
use the same HYPRE installation as used by MOOSE within nekRS, we maintain a separate
[nekRS fork](https://github.com/neams-th-coe/nekRS). Another purpose of this separate fork is
to allow CIVET's recipes to work with nekRS's dependencies. Specifically, nekRS uses
[git subtrees](https://www.atlassian.com/git/tutorials/git-subtree) for its dependencies. However,
Nek5000 contains a [git submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules),
for the `examples` repository containing input file examples.
When CIVET attemps to recursively fetch all nested submodules in Cardinal, the presence of a
submodule inside of a subtree does not work nicely with git's submodule commands. Therefore, our
nekRS fork has deleted the `examples` submodule within nekRS's Nek5000 subtree (this submodule
is not needed anyways, since Cardinal focuses exclusively on nekRS coupling).

Updating the MOOSE, OpenMC, and SAM submodules is routine; this section describes the
steps necessary to update the nekRS submodule; these instructions are solely for project
maintainers - users do not need to manually update submodules.

The nekRS submodule points to the `cardinal` branch on the
[nekRS fork](https://github.com/neams-th-coe/nekRS). Assuming you have a separate repository checkout
of nekRS in your filesystem, first merge the latest master branch into a new nekRS feature update branch.

```
$ pwd
  /home/anovak/nekRS

$ git co -b cardinal-update-5-21
$ git merge origin/master
```

You will need to correct merge conflicts, which should mostly be keeping the
`Deleted by us` changes related to removing the `examples` submodule and retaining
the special HYPRE instructions in `config/hypre.cmake`.
After you have resolved the merge conflicts, open a pull request into the `cardinal` branch
on the [nekRS fork](https://github.com/neams-th-coe/nekRS).

Once the submodule itself is updated, then update the commit pointed to by the nekRS submodule
in Cardinal. During this step, we need to set the values of some nekRS CMake variables that we
don't necessarily set up during Cardinal's build process. In the Cardinal `Makefile`,
update the `NEKRS_VERSION`, `NEKRS_SUBVERSION`, and `GITCOMMITHASH` that are used to
print the nekRS version at the start of the nekRS initialization. The two
nekRS version macros can be found in the `build/nekrs/CMakeCache.txt` file that is created
after compiling Cardinal. Set `NEKRS_VERSION` to the value of `CMAKE_PROJECT_VERSION_MAJOR`
and set `NEKRS_SUBVERSION` to the value of `CMAKE_PROJECT_VERSION_MINOR`. Finally, set
`GITCOMMITHASH` to the [git hash](https://github.com/neams-th-coe/nekRS/commits/cardinal)
of the nekRS fork that you just updated to. All three of `NEKRS_VERSION`, `NEKRS_SUBVERSION`,
and `GITCOMMITHASH` are added to `CXXFLAGS`.
