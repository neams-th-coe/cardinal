# Troubleshooting OpenMC at Runtime

This page collects some common issues encountered when running OpenMC
models.

## ModuleNotFoundError: No module named 'openmc'

If you are trying to use OpenMC's Python API to make input files, this error
means that you did not yet install the Python API. Follow the instructions
[here](python_api.md).

## Missing Cross Section Data

If you see an error like:

```
 ERROR: No cross_sections.xml file was specified in materials.xml or in the
        OPENMC_CROSS_SECTIONS environment variable. OpenMC needs such a file to
        identify where to find data libraries. Please consult the user's guide
        at https://docs.openmc.org/ for information on how to set up data
        libraries.
```

this means that you need to tell OpenMC where to find nuclear data cross sections.
OpenMC supports many different HDF5-format cross section
libraries. Cardinal's test suite assumes that you are using a specific data set
which has data for temperatures between 250 K and 2500 K.
You can get this cross section data set by running

```
$ ./scripts/download-openmc-cross-sections.sh
```

This script will place cross section data into a directory
name `cross_sections` one directory "above" the Cardinal repository.

If you would prefer other libraries, please refer to the
[OpenMC cross section documentation](https://docs.openmc.org/en/stable/usersguide/cross_sections.html).

## Running out of Memory

OpenMC replicates the geometry and material data on every MPI rank. For large problems
(especially TRISO cases with lots of surfaces), you might run out of memory if MOOSE is
also consuming a lot of memory (for instance, if the `[Mesh]` is really large). This will
typically manifest as a crash of the simulation, usually before any time steps have run.

Cardinal supports distributed mesh, allowing the `[Mesh]` to be split among MPI ranks.
This is usually sufficient to address memory issues. See the MOOSE [mesh splitting](https://mooseframework.inl.gov/syntax/Mesh/splitting.html)
documentation for more information on how to use this feature.
