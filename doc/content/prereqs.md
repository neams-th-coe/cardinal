# How to tell if I have the prerequisites for building?

Depending on whether you are building with NekRS and/or OpenMC, Cardinal
has a few prerequisites.

## CMake

Both OpenMC and NekRS use CMake for building.
To check if you have CMake, you should be able to print a version number:

```
cmake --version
```

which will print something like

```
cmake version 3.23.2

CMake suite maintained and supported by Kitware (kitware.com/cmake).
```

If `cmake --version` does not print anything, you need to obtain CMake. If you're
on a cluster, you can do a command like `module spider cmake` to search for CMake
modules and then load one.

If you are on a personal computer, [download CMake](https://cmake.org/download/)
and then make sure that the CMake binary is on your `PATH`.

## MPI

Both NekRS and OpenMC use MPI parallelism. To check if you have MPI, you should be
able to print the versions of various MPI compiler wrappers on your system. For the
C++ wrapper:

```
mpicxx --version
```

should print something like:

```
g++ (Ubuntu 9.4.0-1ubuntu1~20.04.1) 9.4.0
Copyright (C) 2019 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE
```

For the C wrapper:

```
mpicc --version
```

should print something like:

```
gcc (Ubuntu 9.4.0-1ubuntu1~20.04.1) 9.4.0
Copyright (C) 2019 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

And for the Fortran wrapper:

```
mpif90 --version
```

should print something like:

```
GNU Fortran (Ubuntu 9.4.0-1ubuntu1~20.04.1) 9.4.0
Copyright (C) 2019 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

If any of the above does not print something, you need to get MPI wrappers.
If you are on a cluster, you can try commands like `module spider mpich` for
the MPICH implementation, `module spider openmpi` for the OpenMPI implementation,
`module spider intel` for the Intel implementation, etc. Then, load the module you want.

If you are on a personal
computer, try [downloading MPICH](https://www.mpich.org/downloads/),
[download OpenMPI](https://www.open-mpi.org/software/ompi/v4.1/),
or [download Intel MPI](https://www.intel.com/content/www/us/en/developer/tools/oneapi/mpi-library.html#gs.dp8jz3).

Then, it's a good idea to explicitly set in your `~/.bashrc` that you want to use
the MPI compiler wrappers:

```
export CC=mpicc
export CXX=mpicxx
export FC=mpif90
```

## GNU Fortran compilers

NekRS currently requires the GNU Fortran compilers. To check which Fortran compilers
you are using, try:

```
mpif90 --version
```

which will print something like

```
GNU Fortran (Ubuntu 9.4.0-1ubuntu1~20.04.1) 9.4.0
Copyright (C) 2019 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

The `GNU Fortran` part indicates that this is a GNU Fortran compiler. You will
want to be sure that you have set `export FC=mpif90` in order to instruct Cardinal
to use this specific compiler.

If NekRS still complains `Cannot find a supporter compiler!` even though you know
you are using `gfortran`, try:

```
export MPICH_FC=gfortran
```

## HDF5

OpenMC requires HDF5 for cross section data and writing output files.
If you are using MOOSE's conda environment, please set:

```
export HDF5_ROOT=$CONDA_PREFIX
```

If you are instead building PETSc and libMesh via the scripts, MOOSE
will automatically download it for you when
building PETSc. If you don't want to use this auto-download and are on a
cluster, trying `module spider hdf5` and then load a module.

If you are on a personal computer, try:

```
cd /
find . -name "libhdf5.*"
```

which will print out where you have the HDF5 libraries. Then, you should set
`HDF5_ROOT` to the top-level directory that contains the `include` and `lib`
HDF5 directories.
