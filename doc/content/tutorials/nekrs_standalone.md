# Tutorial 1A: Running NekRS as a Standalone Application

In this tutorial, you will learn how to:

- Run NekRS as a standalone application completely separate from Cardinal
- Run a thinly-wrapped NekRS simulation without any physics coupling

This tutorial provides a description for how to use Cardinal to run both
entirely-standalone NekRS cases as well as thinly-wrapped NekRS inputs to
leverage Cardinal's postprocessing and I/O features. Each of these capabilities
is described separately, beginning with the full-standalone case.

## Standalone Simulations
  id=standalone

As part of Cardinal's build process, the `nekrs` executable used to run standalone
NekRS cases is compiled and placed in the `$NEKRS_HOME/bin` directory. In this directory
are also all the usual scripts used to simplify the use of the `nekrs` executable.
To use these scripts to run standalone NekRs cases,
we recommend adding this location to your path:

```
$ export PATH=$NEKRS_HOME/bin:$PATH
```

Then, you can run any standalone NekRS case simply by having built Cardinal -
no need to separately build and compile NekRS. For instance, try running the
`ethier` example that ships with NekRS:

```
$ cd contrib/nekRS/examples/ethier
$ nrsmpi ethier 4
```

which will run the `ethier` case with 4 MPI processes.

## Thinly-Wrapped Simulations

Cardinal offers many convenient features for running [!ac](CFD) simulations that
are not available directly in NekRS. Notable features include:

- Postprocessing operations to evaluate max/mins, area/volume integrals and averages,
  and mass flux-weighted side integrals of various quantities in the [!ac](CFD) solution
- Extracting the NekRS solution onto an any output format supported by MOOSE (such as
  Exodus and VTK - see the full list of formats supported by MOOSE
  [here](https://mooseframework.inl.gov/syntax/Outputs/index.html)).

If your main intention is *not* to couple NekRS to MOOSE, then Cardinal can be used
simply to provide extra postprocessing and data I/O features not available in NekRS
by creating a "thin" wrapper input file that essentially just runs NekRS as a MOOSE
application (but allowing usage of the postprocessing and data I/O features of Cardinal).

!alert! construction title=Tutorial in progress
This capability is available, and a tutorial will be added in the near future.
