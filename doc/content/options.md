Then, decide whether you want *both* NekRS and OpenMC, just one,
or neither. Both are enabled by default, but you can
build Cardinal with only the dependencies that you want.
If you do *not*
want to build the NekRS-part of Cardinal, set the following environment variable:

```
export ENABLE_NEK=false
```

Likewise, if you do *not* want to build the OpenMC-part of Cardinal,
set the following environment variable:

```
export ENABLE_OPENMC=false
```

We support the optional usage of [DAGMC](https://svalinn.github.io/DAGMC/)'s CAD-based models in OpenMC.
This capability is off by default, but to build with [!ac](DAGMC) support, set:

```
export ENABLE_DAGMC=yes
```

If you chose to enable DAGMC, we also provide support for [Double-Down](https://double-down.readthedocs.io/en/latest/) -
a mixed-precision interface to the ray-tracing library [Embree](https://www.embree.org/). Embree support is enabled by
default in Cardinal due to the large improvements to threaded scalability and per-thread performance it provides over
MOAB's native raytracing. If you do *not* want to use DAGMC with Embree, set:

```
export ENABLE_DOUBLE_DOWN=no
```
