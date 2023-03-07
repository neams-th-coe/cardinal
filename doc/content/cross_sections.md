# Getting Cross Section Data

OpenMC requires cross section data in HDF5 format.
Cardinal's test suite assumes that you are using a specific data library
that you can obtain by running

```
./scripts/download-openmc-cross-sections.sh
```

This script will place cross section data into a directory
name `cross_sections` one directory "above" the Cardinal repository.
For other libraries, please refer to the
[OpenMC cross section documentation](https://docs.openmc.org/en/stable/usersguide/cross_sections.html).
