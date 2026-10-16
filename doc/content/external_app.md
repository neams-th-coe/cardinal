# Dynamically Linking to an External MOOSE App

Instead of building another MOOSE application as a Cardinal submodule, you can
couple Cardinal to a separately built (e.g. pre-installed) application by loading
that application's library at runtime with the `library_path` MultiApp parameter
(see also the [other apps tutorial](tutorials/other_apps.md)). This page uses the
[TMAP8](https://github.com/idaholab/TMAP8) module on INL's HPC systems as an example,
but the same steps apply to any MOOSE application.

!alert! warning title=Use the same MOOSE version
Cardinal and the external application *must* be built against the same MOOSE
version. You therefore need to check out the Cardinal commit whose `contrib/moose`
submodule matches the MOOSE version used by the application you load.
!alert-end!

## Find the Matching Cardinal Commit

Pre-built app modules are named `<app>/<date>-<app commit>`, e.g.
`tmap8-openmpi/2026.08.13-ea97ed7` is dated 2026-08-13. Cardinal updates its MOOSE
submodule regularly, so pick the *most recent* MOOSE update on or before the module's date.
These updates are listed in the
[contrib/moose commit history](https://github.com/neams-th-coe/cardinal/commits/devel/contrib/moose),
or from your Cardinal clone with:

``` language=bash
git log --format="%H %cd %s" --date=short -- contrib/moose
```

For the TMAP8 module above, this is commit `9f003d56e80bf69edcb3901fc163b02c8e6e6e5e`
(2026-08-13).

## Build Cardinal

Load the app's module and enter its environment (`tmap8-shell`), so that Cardinal
builds against the same libMesh and PETSc. Then check out the Cardinal commit found
above, so that Cardinal uses the same MOOSE version as the app, and build:

``` language=bash
module purge
module load use.moose
module load tmap8-openmpi/2026.08.13-ea97ed7
tmap8-shell

cd cardinal
git checkout 9f003d56e80bf69edcb3901fc163b02c8e6e6e5e # same MOOSE version as the tmap8 module
./scripts/get-dependencies.sh
export ENABLE_NEK=false
export HDF5_ROOT=${PETSC_DIR}
make -j4 MAKEFLAGS=-j4
```

## Run

Run `cardinal-opt` from the same environment you built it in (`tmap8-shell`). Inside
this environment the app is installed under `/opt/tmap8`. `library_path` must point to
the directory that contains the app's `.la` library archive:

``` language=bash
find /opt/tmap8 -name 'libtmap8-opt.la'
/opt/tmap8/lib/tmap8/libtmap8-opt.la
```

Use that directory as `library_path` in the Cardinal input file, and set `app_type` to
the external app:

```
[MultiApps]
  [tmap8]
    type = FullSolveMultiApp
    app_type = TMAP8App
    input_files = tmap8.i
    library_path = '/opt/tmap8/lib/tmap8'
  []
[]
```

Then run `cardinal-opt` from the same environment you built it in (`tmap8-shell`).
