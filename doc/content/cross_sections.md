# Getting Cross Section Data

OpenMC requires cross section data in HDF5 format.
Cardinal's test suite assumes that you are using a specific data library
that you can obtain by running

```
./scripts/download-openmc-cross-sections.sh
```

This script will place cross section data into a directory
name `cross_sections` one directory "above" the Cardinal repository, so that you'd set
`OPENMC_CROSS_SECTIONS` to something like the following (if you have the Cardinal
repository located at `$HOME/cardinal`):

```
export OPENMC_CROSS_SECTIONS=$HOME/cross_sections/endfb-vii.1-hdf5/cross_sections.xml
```

For other libraries, please refer to the
[OpenMC cross section documentation](https://docs.openmc.org/en/stable/usersguide/cross_sections.html).
