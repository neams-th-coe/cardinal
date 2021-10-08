# Building with THM

THM is a tool for 1-D thermal-hydraulics analysis.
To get access to THM, please first apply through the
[National Reactor Innovation Center](https://ncrcaims.inl.gov/Identity/Account/Login).
To couple Cardinal to THM, you can either maintain separate builds of THM
and Cardinal and use the `library_path` feature of the
[TransientMultiApp](https://mooseframework.inl.gov/source/multiapps/TransientMultiApp.html)
to dynamically load the THM/Cardinal library. Alternatively, you can
use the THM submodule in Cardinal and directly run THM input files using the
Cardinal executable.
To get the THM submodule, run:

```
$ git submodule update --init contrib/thm
```

You will then be prompted to enter credentials to access INL's gitlab site.
