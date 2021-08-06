# HPC Systems

The Cardinal development team regularly builds and runs Cardinal on a number
of [!ac](HPC) systems. To help streamline the compilation
process for other users, this section collects instructions for the modules and any
systems-specific settings for a variety of [!ac](HPC) systems. Because default modules and
settings change on [!ac](HPC) systems with time, the instructions below may become deprecated.
The date at which the settings were last tested is given for each system.
The absence of a particular [!ac](HPC) system from this list does not imply that Cardinal will not
build/run on that system - only that no instructions have yet been documented for it.
Also please note that in addition to these provided module and environment settings,
you must also follow the build instructions on the
[Getting Started](start.md) page.

## LCRC at ANL (5/24/2021)

The following `~/.bashrc` allows you to compile Cardinal on
[LCRC](https://www.lcrc.anl.gov/systems/resources/) at ANL. Please
change the `NEKRS_HOME` variable as appropriate for your build location.

```
if [ -f /etc/bashrc ]; then
        . /etc/bashrc
fi

export NEKRS_HOME=$HOME/cardinal/install

module purge
module load gcc/8.2.0-g7hppkz
module load intel-mpi/2018.4.274-ozfo327
module load intel-mkl/2018.4.274-2amycpi
module load cmake/3.14.2-gvwazz3
module load hdf5/1.8.16-mz7lmxh
module load anaconda3

export CC=mpicc
export CXX=mpicxx
export FC=mpif90
```

## Sawtooth at INL (4/29/2021)

The following `~/.bashrc` allows you to compile Cardinal on Sawtooth. Please
change the `NEKRS_HOME` variable as appropriate for your build location.

```
if [ -f /etc/bashrc ]; then
        . /etc/bashrc
fi

export NEKRS_HOME=$HOME/cardinal/install

module purge
module load openmpi/4.0.5_ucx1.9
module load cmake/3.16.2-gcc-9.3.0-tza7
module load hdf5/1.12.0_ucx1.9

export CC=mpicc
export CXX=mpicxx
export FC=mpif90
```

## Summit at ORNL (8/1/2021)

The following `~/.bashrc` allows you to compile Cardinal on Summit. Please
change the `NEKRS_HOME` variable as appropriate for your build location.

```
module load gcc cmake cuda hdf5 python/3.7.0-anaconda3-5.3.0

export NEKRS_HOME=/gpfs/alpine/csc404/proj-shared/novak/cardinal/install
```
