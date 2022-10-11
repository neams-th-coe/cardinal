The command to run Cardinal with `<n>` MPI ranks and `<s>` OpenMP threads is:

```
mpiexec -np <n> cardinal-opt -i input.i --n-threads=<s>
```

This command assumes that `cardinal-opt` is located on your `PATH`; otherwise,
you need to provide the full path to `cardinal-opt` in the above command.
Note that while MOOSE and OpenMC use hybrid parallelism with both MPI and OpenMP,
NekRS does not use shared memory parallelism.

!alert! tip title=Command line options
Cardinal supports all of MOOSE's command line parameters, as well as a few Cardinal-specific
command line options. For a full list:

```
./cardinal-opt --help
```
!alert-end!

!alert! note title=Running with OpenMC?

- Follow [these instructions](cross_sections.md) to obtain cross section data.
- You may also *optionally* use OpenMC's Python [!ac](API) to build models. To use
  this [!ac](API), follow
  [these instructions](https://docs.openmc.org/en/stable/usersguide/install.html#installing-python-api).

!alert-end!

!alert! note title=Running with NekRS?

Follow [these instructions](nek_tools.md) to obtain binary executables to use for common NekRS-related operations, such as:

- Converting between an Exodus mesh and NekRS's custom `.re2` mesh format
- Generating metadata files for visualizing NekRS's custom field fiel output in Paraview

!alert-end!
