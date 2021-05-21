# Cardinal

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

which will override any other settings. If you plan to use a GPU  backend, then you will also need to
set the correct threading API in the `Makefile` by setting
the values of the `OCCA_CUDA_ENABLED`, `OCCA_HIP_ENABLED`, or `OCCA_OPENCL_ENABLED` variables,
respectively.

## Building

First, fetch all the submodules containing the MOOSE, nekRS, OpenMC, and SAM dependencies. If you
will *not* be using the SAM submodule within Cardinal, you will need to individually clone
only the open-source the submodules:

```
$ git submodule update --init contrib/nekRS
$ git submodule update --init contrib/moose
$ git submodule update --init --recursive contrib/openmc
```

If you are using SAM, you will then also need to fetch the SAM submodule:

```
$ git submodule update --init contrib/SAM
```

You will be prompted for login credentials to ANL's xgitlab site. If you do not have credentials,
you will need to contact the SAM development team to request access.

We recommend fetching the submodules in this individual approach, but you can also
get all submodules at once with

```
$ git submodule update --init --recursive
```

though this may be slow in cloning the nested sub-submodules in native MOOSE-based applications.

After obtaining all necessary submodules, the first software that must be built is the PETSc numerics library:

```
$ ./contrib/moose/scripts/update_and_rebuild_petsc.sh
```

If a MOOSE-compatible version of PETSc is already installed, it can
be used instead of the contrib version, after setting the environment
variables `PETSC_DIR` and `PETSC_ARCH` to the PETSc install directory
and architecture name.

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

Next, you must set the environment variable `NEKRS_HOME` to be the location of the 
nekRS root directory so that all needed include files are on your path.
This will be the `install/` directory under the top-level Cardinal directory:

```
$ export NEKRS_HOME=$(realpath install/)
```

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

### Optional Make Variables

* `HDF5_INCLUDE_DIR`: Specify location of HDF5 headers.  Defaults to:
  * `$HDF5_ROOT/include` if `HDF5_ROOT` is specified
  * `/usr/include` if `HDF5_ROOT` is not specified
* `HDF5_LIBDIR`: Specify location of HDF5 libraries.  Defaults to:
  * `$HDF5_ROOT/lib` if `HDF5_ROOT` is specified
  * `/usr/lib` if `HDF5_ROOT` is not specified

## Running

Enter the directory with your nekRS case files and use a command such as:

```
$ mpirun -np 4 ~/repos/cardinal/cardinal-opt --app nek -i nek.i --nekrs-setup onepebble2
```

where `--nekrs-setup` is the basename of the nekRS files for your case. The `-app` may
be one of `nek`, `openmc`, or `cardinal` (the default). The `-app` flag basically registers
the objects in Cardinal under different MooseApps, reflecting how these objects would
interact with each other if the OpenMC and nekRS wrappings were each ported out to individual
apps, rather than the coupled case here for Cardinal.
