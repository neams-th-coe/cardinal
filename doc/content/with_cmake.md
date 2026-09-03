# Building Cardinal with CMake

!alert! warning title=Experimental
This CMake-based build is an *experimental* alternative to Cardinal's normal Makefile-based
build, described in [the instructions for building without MOOSE's conda environment](without_conda.md)
(and, for the OpenMC-only conda workflow, [with_conda.md](with_conda.md)). It has +not+
replaced either of those as Cardinal's primary, best-supported build system, and it does +not+
replace or depend on MOOSE's own (separately in-progress) CMake port. If you run into trouble here,
or just want the most battle-tested path, please use [without_conda.md](without_conda.md) instead.
See [cmake_details.md](cmake_details.md) for the design and implementation notes behind this build.
!alert-end!

!alert! note title=tldr

On *CPU systems*, all that you need to compile Cardinal is:

```
cd $HOME
git clone https://github.com/neams-th-coe/cardinal.git
cd cardinal
cmake -S . -B build
cmake --build build -j8
export NEKRS_HOME=$HOME/cardinal/build/install
```

If the above produces a `build/cardinal-opt` executable, you can
jump straight to [#running]. If you are on a GPU system, want to customize the
build, or were not successful with the above, please consult the detailed instructions
that follow.
!alert-end!

## Access

!include access.md

## Prerequisites

!include cardinal_prereqs.md

!alert! tip title=How do I know if I have these dependencies?
Most systems will already have these available.
To figure out if you have these dependencies, check out
[our prerequisite guide](prereqs.md). You will additionally need CMake 3.21 or newer
(`cmake --version`).
!alert-end!

## How This Differs from the Makefile Build
  id=overview

Cardinal's own compile (`framework/build.mk`, `app.mk`, etc, and its `config/*.mk` dependency
rules) is unchanged and still does the real work. This CMake project is a
*superbuild*: a separate, out-of-source `CMakeLists.txt` at the top level of the repository whose
only job is to resolve and build the dependencies that have their own CMake sub-build (NekRS,
OpenMC, and -- for [!ac](DAGMC) support -- MOAB, Embree, and Double-Down), then hand off to
Cardinal's existing, unmodified `Makefile` to compile Cardinal itself. A few things follow from that:

- +The build directory *is* the equivalent of a normal Cardinal checkout.+ Running
  `cmake -S . -B build` mirrors your whole source tree into `build/` (excluding the dependencies
  managed below), lays out each dependency exactly where the Makefile already expects it, and then
  `cmake --build build` invokes `make` there for you. The result looks just like an in-place
  Makefile build: `build/cardinal-opt`, `build/lib/`, `build/contrib/`, etc, all directly under
  `build/` -- not nested inside an extra `cardinal/` subdirectory. Because of this, you should treat
  `build/` as a real (if regeneratable) Cardinal checkout, e.g. `NEKRS_HOME` below points *into*
  it, and `cardinal-opt` needs to be run from there.
- +No `./scripts/get-dependencies.sh` step.+ Configuring resolves MOOSE/NekRS/OpenMC/MOAB/
  Embree/Double-Down/[!ac](DAGMC)/`nuclear_data` automatically: if you already have a submodule
  checked out under `contrib/`, it's used (mirrored into `build/` on every build); otherwise it's
  cloned directly into the build tree at the exact pinned commit from `.gitmodules`. Either way,
  your own source checkout under `contrib/` is never modified.
- +PETSc/libMesh/WASP are built from source automatically if you don't already have them.+
  Unlike the Makefile workflow, there's no separate `update_and_rebuild_{petsc,libmesh,wasp}.sh`
  step to run yourself -- `cmake --build build` does it as part of the same build, using those
  same scripts under the hood. If you already have any of them built (e.g. a system/module-provided
  PETSc), point at it instead with `-DPETSC_DIR=...` etc (see [#env] below) and that one is skipped.
- +Options are CMake cache variables, not environment variables+ -- set with `-D<option>=<value>`
  at configure time, or interactively with `ccmake build` (toggle values, `c` to configure, `g` to
  generate, then `cmake --build build`). See [#options].
- +`CMAKE_INSTALL_PREFIX` is a real, standard CMake install prefix+ -- untouched by the ordinary
  build (`cmake --build build`), exactly like any other CMake project. Nothing lands there until
  you explicitly ask, with `cmake --build build --target install`; see [#running] below.

## Configuration Options
  id=options

Decide whether you want NekRS, OpenMC, both (the default), or neither:

```
cmake -S . -B build -DENABLE_NEK=OFF     # skip the NekRS-part of Cardinal
cmake -S . -B build -DENABLE_OPENMC=OFF  # skip the OpenMC-part of Cardinal
```

We support the optional usage of [DAGMC](https://svalinn.github.io/DAGMC/)'s CAD-based models in
OpenMC. This capability is off by default, but to build with [!ac](DAGMC) support, set:

```
cmake -S . -B build -DENABLE_DAGMC=ON
```

!alert! note title=Performance Improvement with Double-Down
If you chose to enable DAGMC, we also provide support for [Double-Down](https://double-down.readthedocs.io/en/latest/) -
a mixed-precision interface to the ray-tracing library [Embree](https://www.embree.org/). Embree support is disabled by
default, but we *strongly* recommend enabling it - you can expect to see around 100x speedup when using Double-Down
on mesh-based geometries.

If you do want to use DAGMC with Embree, set:

```
cmake -S . -B build -DENABLE_DAGMC=ON -DENABLE_DOUBLE_DOWN=ON
```
!alert-end!

!alert! note title=Optional dependencies not managed here
SAM, Sockeye, and BISON coupling (see [dependencies.md](dependencies.md)) are out of scope for this
CMake build -- none of them have their own CMake sub-build the way NekRS/OpenMC/MOAB/Embree/
Double-Down/[!ac](DAGMC) do, and several require INL/ANL-internal credentials just to check out. If
you need one of these, use the [Makefile-based workflow](without_conda.md) instead.
!alert-end!

Any of these can also be toggled interactively:

```
ccmake build
```

## Building
  id=build

#### Set Environment Variables
  id=env

Two environment variables are relevant when *running* Cardinal built this way (the build itself
does not need them set -- see [#overview] above). Put these in your `~/.bashrc`:

``` language=bash
# [REQUIRED] location of the NekRS install produced by the build -- note this is
# *inside* the CMake build directory, not the top-level repository checkout.
export NEKRS_HOME=$HOME/cardinal/build/install

# [OPTIONAL] if running with OpenMC, you will need cross section data at runtime;
# you will need to set this variable to point to a 'cross_sections.xml' file.
export OPENMC_CROSS_SECTIONS=${HOME}/cross_sections/endfb-vii.1-hdf5/cross_sections.xml
```

If you want to control which compilers are used (rather than letting the configure step pick them
up from a pre-built `LIBMESH_DIR`, or find `mpicc`/`mpicxx`/`mpif90` on `PATH`), set `CC`/`CXX`/`FC`
in the environment *before* configuring, the same as the Makefile workflow.

#### GPU/OCCA Backend
  id=gpu

NekRS's device backends (CUDA, HIP, OpenCL, SYCL/DPCPP, Metal) are all off by default -- matching
what a container build with no GPU toolkits installed would produce -- and are enabled with their
own options:

```
cmake -S . -B build -DENABLE_CUDA=ON   # or -DENABLE_HIP=ON, -DENABLE_OPENCL=ON, ...
```

!alert! warning title=NekRS's bundled HYPRE and CUDA 13
NekRS builds a GPU-resident variant of its bundled HYPRE (used for the multigrid coarse solve)
whenever a device backend is enabled, controlled by `ENABLE_HYPRE_GPU` (default `ON`). The
vendored HYPRE version does not build against CUDA &ge;13.3 (a real upstream incompatibility,
also present in Cardinal's native Makefile build against the same CUDA version, unrelated to
this CMake build) -- if you hit this, configure with `-DENABLE_HYPRE_GPU=OFF` instead. HYPRE
still runs correctly, just on the CPU, while everything else remains GPU-resident; this mirrors
[the equivalent Frontier (HIP) workaround](hpc.md) already documented for the native build.
See [cmake_details.md](cmake_details.md) for the full story, including validation on real GPU
hardware.
!alert-end!

#### Build PETSc, libMesh, and WASP
  id=petsc_libmesh

By default, `cmake --build build` builds all three of PETSc, libMesh, and WASP from source for
you (see [#overview] above), so there is nothing extra to run. Each can instead point at an
existing install:

```
cmake -S . -B build -DPETSC_DIR=/opt/petsc -DLIBMESH_DIR=/opt/libmesh -DWASP_DIR=/opt/wasp
```

`PETSC_DIR`, `LIBMESH_DIR`, and `WASP_DIR` are independent of each other, with one exception:
a pre-built `LIBMESH_DIR` was compiled against some specific PETSc, so `LIBMESH_DIR` requires
`PETSC_DIR` to also be given. `HDF5_ROOT` defaults to a location derived from `PETSC_DIR`
(matching the Makefile workflow's own default) and can also be set explicitly if needed.

If you're building one of these from source and need to pass it something not already covered
by another CMake option (an extra `--with-...` PETSc configure flag, say), `PETSC_SCRIPT_ARGS`,
`LIBMESH_SCRIPT_ARGS`, and `WASP_SCRIPT_ARGS` are appended to that dependency's own
`update_and_rebuild_*.sh` configure run, e.g.
`-DPETSC_SCRIPT_ARGS="--with-debugging=0 --download-slepc"`.

!alert tip
Building PETSc/libMesh from source is just as time consuming here as in the Makefile workflow.
Set `-DMOOSE_BUILD_PARALLELISM=<N>` (defaults to `CMAKE_BUILD_PARALLEL_LEVEL` from the
environment, else `min(available cores, 16)`) to control how parallel *that* part of the build
is -- independent of the `-j<N>` you pass to `cmake --build`, since these scripts can't share
that jobserver.
!alert-end!

#### Compile Cardinal
  id=compiling

```
cmake --build build -j8
```

This mirrors your source into `build/`, resolves/builds whichever dependencies are needed, and
finally invokes Cardinal's own `Makefile` there, all in one command -- always at whatever
parallelism `-j` you asked for. This produces the executable `build/cardinal-<mode>`, where
`<mode>` is `opt` normally, or `dbg` if configured with `-DCMAKE_BUILD_TYPE=Debug`. If you
encounter issues while compiling, check out our [compile-time troubleshooting guide](compiletime.md)
(most of which still applies -- the actual compile step is Cardinal's ordinary Makefile build).

## Running
  id=running

!include running.md

Note that, per [#overview] above, `cardinal-opt` here lives in `build/`, not in the top-level
repository checkout.

!alert note title=Installing
`build/` isn't meant to be kept around forever, but by default nothing needed to actually *run*
Cardinal -- `cardinal-opt` itself, the MOOSE framework/module libraries it links, and
nekRS/OpenMC/MOAB/Embree/Double-Down/[!ac](DAGMC)/`nuclear_data` -- exists anywhere else.
Running:

```
cmake --install build --prefix /path/to/install
```

(equivalently, `cmake --build build --target install`, or set `CMAKE_INSTALL_PREFIX` at configure
time with `--install-prefix /path/to/install`; default is `build/dist` -- see [#options] above)
copies all of it into `/path/to/install`, rewriting RPATHs so the result no longer depends on
`build/` at all -- safe to run from `/path/to/install/bin/cardinal-opt` even after deleting
`build/` entirely. Laid out to look like `cardinal-opt`'s own checkout, not a pile of every
dependency's files merged together: `cardinal-opt`, its own `lib/`, and `share/` sit directly
under `/path/to/install`, while nekRS/OpenMC/MOAB/Embree/Double-Down/DAGMC/`nuclear_data` --
which install as one shared tree, matching the native Makefile build's own `CARDINAL_DIR/install`
-- go into `/path/to/install/install`. Nothing is copied there until you ask for it: like any
other CMake project, `CMAKE_INSTALL_PREFIX` is left alone during the ordinary build, and changing
it (`ccmake` included) never invalidates anything already built.

Installing directly into your source checkout (`cmake --install build --prefix $(pwd)`, from
inside the checkout) additionally symlinks `cardinal-opt` at the checkout root to
`bin/cardinal-opt`, matching where the native Makefile build puts it -- skipped for any other
install prefix, where there's no checkout layout for it to be matching.
!alert-end!

## Checking the Install

!include checking_install.md
