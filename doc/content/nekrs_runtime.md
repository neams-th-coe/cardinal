# Troubleshooting NekRS at Runtime

This page collects some common issues encountered when running NekRS
models. Generally, if you are encountering issues it is a good idea to
*clear the cache* directory so that you're absolutely sure there aren't
old case build files lingering around that are causing conflicts. Try
the following first:

```
rm -rf .cache/
```

and then re-run your case. If the issue persists, check out these other
potential causes.

## MPI_Abort

If a NekRS-wrapped case immediately fails with just an MPI Abort, like this:

```
using NEKRS_HOME: /home/anovak/cardinal/install
using NEKRS_CACHE_DIR: /home/anovak/cardinal/tutorials/fhr_reflector/conduction/.cache
using OCCA_CACHE_DIR: /home/anovak/cardinal/tutorials/fhr_reflector/conduction/.cache/occa/

application called MPI_Abort(MPI_COMM_WORLD, 1) - process 0
```

this usually means that you have an error in your NekRS case files (like an invalid
parameter). Unfortunately, NekRS does not always propagate these errors upwards to the
console, so it's impossible to diagnose what actually went wrong if you only see an
MPI Abort. You can figure out what the error is by instead just running the `nrsmpi`
executable, like

```
nrsmpi case 4
```

which *will* display the root cause of the MPI Abort. We have an
[issue](https://github.com/neams-th-coe/cardinal/issues/496) to resolve this in the future.

## Insufficient Memory

If you encounter an error like the following

```
obj/libnek5000.a(navier1.o): In function `expl_strs_e_':
navier1.f:(.text+0x11a3): relocation truncated to fit: R_X86_64_PC32 against `.bss'
```

this means that you have likely run out of memory. Please increase the number of nodes.
In some cases, you might need to delete the `.cache` directory for the new
pre-compilation to occur correctly.

## Threading

NekRS does not support threading. On some systems, you might see an error like

```
libgomp: Thread creation failed: Resource temporarily unavailable
```

This error can usually be addressed by explicitly telling NekRS to use just 1 OpenMC
thread, with `export OMP_NUM_THREADS=1`.
