# Building with SAM

SAM is a systems analysis tool for advanced non-light water reactor analysis.
To get access to SAM, please first contact the
[SAM development team](https://www.anl.gov/nse/system-analysis-module).
To couple Cardinal to SAM, you can maintain a separate build of SAM
and Cardinal and use the `library_path` feature of the
[TransientMultiApp](https://mooseframework.inl.gov/source/multiapps/TransientMultiApp.html)
to dynamically load the SAM/Cardinal library (see [this tutorial](tutorials/other_apps.md).
Alternatively, you can
use the SAM submodule in Cardinal and directly run SAM input files using the
Cardinal executable. To get the SAM submodule, run:

```
$ git submodule update --init contrib/SAM
```

You will then be prompted to enter credentials to access ANL's gitlab site.
