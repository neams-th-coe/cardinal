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
