# Building with BISON

BISON is a tool for the modeling of nuclear fuels.
To get access to BISON, please first apply through the
[National Reactor Innovation Center](https://ncrcaims.inl.gov/Identity/Account/Login).
To couple Cardinal to BISON, you can either maintain separate builds of BISON
and Cardinal and use the `library_path` feature of the
[TransientMultiApp](https://mooseframework.inl.gov/source/multiapps/TransientMultiApp.html)
to dynamically load the BISON/Cardinal library. Alternatively, you can
use the BISON submodule in Cardinal and directly run BISON input files using the
Cardinal executable.
To get the BISON submodule, run:

```
./scripts/get-bison.sh
```

You will then be prompted to enter credentials to access INL's gitlab site.
