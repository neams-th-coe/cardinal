The command to run Cardinal with an input file `input.i`, `<n>` MPI ranks, and `<s>` OpenMP threads is:

```
mpiexec -np <n> cardinal-opt -i input.i --n-threads=<s>
```

This command assumes that `cardinal-opt` is located on your `PATH`; otherwise,
you need to provide the full path to `cardinal-opt` in the above command or
add the `cardinal` folder [to your path](https://phoenixnap.com/kb/linux-add-to-path).
Note that while MOOSE and OpenMC use hybrid parallelism with both MPI and OpenMP,
NekRS does not use shared memory parallelism.

!alert! tip title=Command line options
Cardinal supports all of MOOSE's command line parameters, as well as a few Cardinal-specific
command line options. For a full list:

```
./cardinal-opt --help
```
!alert-end!
