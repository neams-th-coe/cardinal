Finally, run `make` in the top-level directory,

```
make -j8 MAKEFLAGS=-j8
```

which will compile Cardinal in parallel with 8 processes
(the `MAKEFLAGS` part is optional, but will also tell CMake to build
in parallel with 8 processes - otherwise, the CMake aspects of Cardinal,
i.e. OpenMC, NekRS, and DAGMC, will build serially).
This will create the executable `cardinal-<mode>` in the
top-level directory. `<mode>` is the optimization level used to compile MOOSE
set with the `METHOD` environment variable. If you encounter issues
while compiling, check out our [compile-time troubleshooting guide](compiletime.md).
