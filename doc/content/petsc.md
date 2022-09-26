# Running the PETSc tests

After building PETSc, if you want to test the installation you will need to `cd`
into the PETSc directory before running the on-screen directions that print
when PETSc finishes, i.e. like:

```
cd contrib/moose/petsc
make PETSC_DIR=$HOME/cardinal/contrib/moose/scripts/../petsc PETSC_ARCH=arch-moose check
```
