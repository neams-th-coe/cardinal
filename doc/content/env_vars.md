# Optional Environment Variables

To figure out what environment variables Cardinal also supports,
open up Cardinal's `Makefile` and check out the environment variables
listed at the beginning.

Some of the more common variables you might want to use include:

- `HDF5_ROOT`: top-level directory containing HDF5 (this directory should contain
   an `include` and a `lib` directory). If not set, this will default to the HDF5 downloaded
   by PETSc.
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
