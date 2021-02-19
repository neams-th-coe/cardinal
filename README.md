# Cardinal

## Building

First, set the correct threading API to use for nekRS (CUDA, HIP, or OpenCL) in `Makefile` by setting
the values of the `OCCA_CUDA_ENABLED`, `OCCA_HIP_ENABLED`, or `OCCA_OPENCL_ENABLED` variables,
respectively. If all of these threading APIs are turned off, then a serial backend will be used
in nekRS, meaning that nekRS will run with MPI parallelism.

Next, fetch all the submodules containing the MOOSE, nekRS, and OpenMC dependencies:

```
$ git submodule update --init --recursive
```

To save some time, especially for fetching the very big `large_problems` submodule or
other large recursive sub-submodules, you can just fetch the minimum number of submodules to
build Cardinal:

```
$ git submodule update --init contrib/nekRS
$ git submodule update --init contrib/moose
$ git submodule update --init --recursive contrib/openmc
```

Next, build the PETSc and libMesh dependencies of MOOSE:

```
$ ./contrib/moose/scripts/update_and_rebuild_petsc.sh
$ ./contrib/moose/scripts/update_and_rebuild_libmesh.sh
```

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
