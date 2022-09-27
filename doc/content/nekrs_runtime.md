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
