# Building Cardinal with MOOSE's conda Environment

!alert! warning title=Incompatible with NekRS
MOOSE's conda environment is currently incompatible with Cardinal's
NekRS wrapping. MOOSE's environment exports some header files that point
to HYPRE, but NekRS needs to use its own special version of HYPRE. The MOOSE
team is not aware of any way to exclude header files attached to MOOSE's
conda compiler wrappers.

You MUST set `export ENABLE_NEK=false` if you want to use the conda environment.
Otherwise, if you do want NekRS features, you need to instead follow the
[instructions for building without the conda environment](without_conda.md).

We hope to relax this restriction in the future - if this
is impacting your work, please let us know so that we can increase its priority.
!alert-end!

If you're only interested in OpenMC, then you're in the right place -
please follow the instructions on this page.

!alert! note title=tldr

On *CPU systems*, all that you need to compile Cardinal is:

```
cd $HOME
git clone https://github.com/neams-th-coe/cardinal.git
cd cardinal
./scripts/get-dependencies.sh
export HDF5_ROOT=$CONDA_PREFIX
export NEKRS_HOME=$HOME/cardinal/install
make -j8
```

If the above produces a `cardinal-opt` Cardinal executable, you can
jump straight to [#running]. If you are on a GPU system, want to customize the
build, or were not successful with the above, please consult the detailed instructions
that follow.
!alert-end!

## Access

!include access.md

## Prerequisites

The basic prerequisites for building Cardinal are summarized in [prereq_table].

!table id=prereq_table caption=Summary of prerequisites needed for Cardinal.
|    | Building with NekRS | Building with OpenMC | Both |
| :- | :- | :- | :- |
| CMake | $\checkmark$ | $\checkmark$ | $\checkmark$ |
| GNU fortran compilers | $\checkmark$ | &nbsp; | $\checkmark$  |
| HDF5 | &nbsp; | $\checkmark$ | $\checkmark$ |
| MPI | $\checkmark$ | $\checkmark$ | $\checkmark$ |

!alert! tip title=How do I know if I have these dependencies?
You will already have all of these if using MOOSE's conda environment.
But if you want to learn more, check out
[our prerequisite guide](prereqs.md).
!alert-end!

!include options.md

## Building
  id=build

#### Fetch Dependencies
  id=fetch

!include dependencies.md

#### Set Environment Variables
  id=env

!include start_env_vars.md

#### Compile Cardinal
  id=compiling

!include compiling.md

## Running
  id=running

!include running.md

## Checking the Install

!include checking_install.md
