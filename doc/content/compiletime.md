# Troubleshooting at Compile-Time

This page collects some common issues encountered when compiling Cardinal.

## Switching 3rd Party Options

If you are toggling `ENABLE_NEK`, `ENABLE_OPENMC` and/or `ENABLE_DAGMC`,
you may need to clear out the `build/` and `install/` directories in-between
changing these options, or else you'll get an error like this:

```
CMake Error at cmake/FindMOAB.cmake:7 (string):
  string sub-command REPLACE requires at least four arguments.
Call Stack (most recent call first):
  CMakeLists.txt:62 (find_package)


CMake Error at cmake/FindMOAB.cmake:17 (message):
  Could not find MOAB.  Set -DMOAB_DIR=<MOAB_DIR> when running cmake or use
  the $MOAB_DIR environment variable.
Call Stack (most recent call first):
  CMakeLists.txt:62 (find_package)
```

We're not exactly sure what the nature of the error is, but have
[an issue](https://github.com/neams-th-coe/cardinal/issues/556)
to track it. If this is affecting your workflow, please contact us so that
we can reprioritize it.


## Linking Errors After Updating Source

Sometimes, despite following all instructions, `make` will not work. For example, if it has been a while since updating Cardinal source, you have cleaned out `build/` and `install/`, and building causes a linking error similar to

```
Linking libpng: -lpng16 -lz
Linking Library cardinal/contrib/moose/framework/libmoose-opt.la...
/usr/bin/ld: cannot find -lptscotchparmetis
collect2: error: ld returned 1 exit status
```

try running `git clean -xfd` from `cardinal/contrib/moose` or wherever you are pointing to MOOSE via the `MOOSE_DIR` variable. If all else fails, clone cardinal in a new location and start the instructions from the beginning. This should prevent any old builds from interfering with the build process. If a fresh clone and rebuild fails, there maybe be environment issues with your system.