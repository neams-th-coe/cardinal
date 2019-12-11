# Cardinal

## Building and Running

### With NekRS

To build with NekRS, set the correct threading API (CUDA, OpenCL, or none) in `Makefile`.  Then, in
the top-level directory, run `make`.  This will create the executable `cardinal-<mode>` in the
top-level directory.

To run with NekRS, first you must export NekRS's configuration variables to your environment.
These are defined in the file `bin/nekrs.bashrc`:

```
$ source bin/nekrs.bashrc
```


Then enter the directory with your NekRS case files and use a command such as:

```
$ mpirun -np 2 ~/repos/cardinal/cardinal-opt --app nek -i nek.i --nekrs-setup eddy
```

where `--nekrs-setup` is the basename of the `.par` and `.usr` files for your case.  

