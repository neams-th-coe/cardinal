Cardinal has MOOSE, OpenMC, and NekRS as dependencies. However, you do not need to separately
build/compile *any* of these dependencies - Cardinal's Makefile handles all steps
automatically. To fetch the MOOSE, OpenMC, and NekRS dependencies, run:

```
./scripts/get-dependencies.sh
```

!alert! note title=Optional dependencies

Cardinal supports *optional* coupling to the following codes:

- *SAM*, a tool for systems analysis of advanced non-light water reactors
  safety analysis. Follow [these instructions](sam_instructions.md) to obtain the required dependencies for adding the
  SAM submodule.
- *Sockeye*, a tool for modeling of heat pipe systems. Follow [these instructions](sockeye_instructions.md) to obtain the required dependencies for adding the
  Sockeye submodule.
!alert-end!
