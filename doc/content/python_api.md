# Installing OpenMC's Python API

OpenMC's Python API is used to build OpenMC's input files (which Cardinal then
couples to MOOSE). General instructions on how
to install the Python API are given on the [OpenMC website](https://docs.openmc.org/en/stable/usersguide/install.html#installing-python-api). If you are using the default
3rd party dependencies in Cardinal, all you'll need to do is

```
cd cardinal/contrib/openmc
pip install .
```
