# Getting Started

The compilation instructions for Cardinal are almost the same as any
other MOOSE application.
First, please read these two important points so that we're on the same page.

!alert! tip title=Setting an Environment Variable
We will ask you to set a few [environment variables](https://www.geeksforgeeks.org/environment-variables-in-linux-unix/) to configure your system.
You can set an environment variable *in a single terminal session* with an *export*
statement, like `export ENABLE_OPENMC=yes`. However, if you were to open a *new* terminal window,
these variables would no longer be set.
If you want these settings to persist *every* time you open a terminal,
you can set them
in your `~/.bashrc`. The very first time you put a new statement in your `~/.bashrc`,
remember that you must *source* the `~/.bashrc` for the changes to take effect!

```
source ~/.bashrc
```
!alert-end!

!alert! tip title=Changing Environment Mid-Build
If you change an environment variable *partway* through building
(i.e. at any time after you have typed `make` or built PETSc/libMesh),
you may need to clear out old build files.
To clear out these files,

```
cd cardinal
rm -rf build/ install/
```

If you're NOT using MOOSE's conda environment, you might also
need to rebuild PETSc and libMesh.
!alert-end!

# Build Instructions

Now that the basics are in place, there are some small differences depending on
whether you want to use MOOSE's conda environment.
Please select your case:

- [I want to use MOOSE's conda environment](with_conda.md)
- [I do NOT want to use MOOSE's conda environment](without_conda.md)
