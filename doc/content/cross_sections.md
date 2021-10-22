# Getting Cross Section Data

OpenMC requires cross section data in HDF5 format.
OpenMC supports many different HDF5-format cross section
libraries. Cardinal's test suite assumes that you are using the ENDF/B7-II.1 data set,
which has data for temperatures between 250 K and 2500 K.
You can get this cross section data set by running

```
$ ./scripts/download-openmc-cross-sections.sh
```

This script will place cross section data into a directory
name `cross_sections` one directory "above" the Cardinal repository.

If you would prefer other libraries, please refer to the
[OpenMC cross section documentation](https://docs.openmc.org/en/stable/usersguide/cross_sections.html).
