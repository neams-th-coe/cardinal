# Cardinal

## Building and Running

### With NekRS

To build with NekRS, set the correct threading API (CUDA, OpenCL, or none) in `Makefile`.  Then, in
the top-level directory, run `make`.  This will create the executable `cardinal-<mode>` in the
top-level directory.

To run with NekRS, first you must set the environment variable `NEKRS_HOME` to be the location of the 
NekRS root directory.  For a Cardinal build, this will be the top-level of the repository.  

```
$ export NEKRS_HOME=$(pwd)
```


Then enter the directory with your NekRS case files and use a command such as:

```
$ mpirun -np 4 ~/repos/cardinal/cardinal-opt --app nek -i nek.i --nekrs-setup onepebble2
```

where `--nekrs-setup` is the basename of the `.par` and `.usr` files for your case.  

### Optional Make Variables

* `HDF5_INCLUDE_DIR`: Specify location of HDF5 headers.  Defaults to:
  * `$HDF5_ROOT/include` if `HDF5_ROOT` is specified
  * `/usr/include` if `HDF5_ROOT` is not specified
* `HDF5_LIBDIR`: Specify location of HDF5 libraries.  Defaults to:
  * `$HDF5_ROOT/lib` if `HDF5_ROOT` is specified
  * `/usr/lib` if `HDF5_ROOT` is not specified
