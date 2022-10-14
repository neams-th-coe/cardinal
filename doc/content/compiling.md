Finally, run `make` in the top-level directory,

```
make -j8
```

which will compile Cardinal in parallel with 8 processes.
This will create the executable `cardinal-<mode>` in the
top-level directory. `<mode>` is the optimization level used to compile MOOSE
set with the `METHOD` environment variable.

```
rm -rf cardinal/install cardinal/build
```
!alert-end!
