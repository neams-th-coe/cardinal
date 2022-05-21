# Building with Sockeye

Sockeye is a tool for the modeling of heat pipe systems.
To get access to Sockeye, please first apply through the
[National Reactor Innovation Center](https://ncrcaims.inl.gov/Identity/Account/Login).
To couple Cardinal to Sockeye, you can maintain a separate build of Sockeye
and Cardinal and use the `library_path` feature of the
[TransientMultiApp](https://mooseframework.inl.gov/source/multiapps/TransientMultiApp.html)
to dynamically load the Sockeye/Cardinal library (see [this tutorial](tutorials/other_apps.md).
Alternatively, you can
use the Sockeye submodule in Cardinal and directly run Sockeye input files using the
Cardinal executable.
To get the Sockeye submodule, run:

```
$ ./scripts/get-sockeye.sh
```

You will then be prompted to enter credentials to access INL's gitlab site.
