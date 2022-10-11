Finally, run `make` in the top-level directory,

```
make -j8
```

which will compile Cardinal in parallel with 8 processes.
This will create the executable `cardinal-<mode>` in the
top-level directory. `<mode>` is the optimization level used to compile MOOSE
set with the `METHOD` environment variable.

!alert! tip
If at any point during the build process, you change your environment, then
you may need to start the build from scratch to be sure that the same
HDF5/compilers/etc. are used to build the entire set of dependencies. When
you restart the build, be sure to clear the `build` and `install`
directories so that the appropriate CMake configuration files will be recreated
with the newest environment.

```
rm -rf cardinal/install cardinal/build
```
!alert-end!
