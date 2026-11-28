# Coupling to Other MOOSE Apps

In this tutorial, you will learn how to:

- Couple Cardinal to an arbitrary MOOSE application with dynamic linking
- Run Cardinal as either the main application or a sub-application

To access this tutorial,

```
cd cardinal/tutorials/other_apps
```

In many cases you may want to couple Cardinal to a separately built MOOSE application by
loading that application's library at runtime. This tutorial couples Cardinal to
[TMAP8](https://github.com/idaholab/TMAP8), a MOOSE application for tritium transport.

## Building

First, follow the [dynamic linking build instructions](external_app.md)
to build Cardinal against the same MOOSE version as your TMAP8 installation.

## Problem Description

OpenMC computes the tritium production rate in a 1 m thick slab of natural lithium,
irradiated on its $x=0$ face by $10^{18}$ 14.1 MeV neutrons per second. The slab is
1 m $\times$ 1 m in cross section with reflective transverse boundaries, so it is
effectively infinite. TMAP8 then solves for tritium diffusion and trapping in the slab,
using the OpenMC tritium production rate as a volumetric source. The coupling is one-way:
the `H3_production` tally (atoms/m$^3$/s) from Cardinal is transferred to the
`tritium_source` variable in TMAP8.

### OpenMC Model

The OpenMC model divides the slab into 20 cells along $x$. Run `python model.py`
to regenerate `model.xml`.

!listing /tutorials/other_apps/model.py language=python

The Cardinal input maps each element of a matching 20-element mesh to one OpenMC cell,
and tallies the tritium production rate in each cell.

!listing /tutorials/other_apps/openmc.i

### TMAP8 Model

The TMAP8 input is the diffusion-limited case of ver-1d (Permeation Problem with Trapping) from the
[TMAP8 tutorial](https://tmap8.inl.gov/verification_and_validation/ver-1d.html), with two changes. The
fixed upstream concentration is replaced by zero concentration on both faces, and a
volumetric source is added from the transferred `tritium_source`, normalized by
`cl` like the concentrations in [ver-1d](https://tmap8.inl.gov/verification_and_validation/ver-1d.html).

!listing /tutorials/other_apps/tmap8.i
  block=AuxVariables Kernels

The trapping kernels in this input (`TrappingNodalKernel` and `ReleasingNodalKernel`)
only exist in TMAP8, so running this input requires the TMAP8 library.

## Running Cardinal as the Main App

To run Cardinal as the main application, we include the Cardinal input and add a
[FullSolveMultiApp](FullSolveMultiApp.md) that runs TMAP8 after OpenMC has solved.
Setting `app_type = TMAP8App` together with a `library_path` that points to the directory
containing `libtmap8-opt.la` tells MOOSE which library to load. The tritium
production rate is then sent to TMAP8 with a
[MultiAppGeneralFieldShapeEvaluationTransfer](MultiAppGeneralFieldShapeEvaluationTransfer.md).

!listing /tutorials/other_apps/cardinal_main.i

Run this input with the Cardinal executable:

```
cardinal-opt -i cardinal_main.i
```

## Running Cardinal as the Sub App

To instead run Cardinal as the sub-application, we include the TMAP8 input and add a
`CardinalApp` MultiApp, with `library_path` now pointing to the `lib` directory in
Cardinal. OpenMC runs once on `initial`, and its tritium production rate is transferred
to TMAP8 before the TMAP8 transient begins.

!listing /tutorials/other_apps/tmap8_main.i

Run this input with the TMAP8 executable, since the main application is now TMAP8:

```
tmap8-opt -i tmap8_main.i
```

!alert note
The main application must always be run with its own executable. `cardinal-opt` cannot
run `tmap8_main.i`, because the TMAP8 objects in the main input are not available until
a TMAP8 library is loaded; likewise, `tmap8-opt` cannot run `cardinal_main.i`.

## Results

In both cases, the `tritium_production` in the Cardinal output (atoms/s) equals the
`production` in the TMAP8 output (atoms/m$^2$/s) because the slab face is 1 m$^2$,
which confirms that the transfer preserved the source. By the end of the TMAP8 transient,
the tritium released through the two faces (`release_left` plus `release_right`) nearly
balances `production`.
