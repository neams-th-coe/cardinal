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
For all of the instructions below, be sure to change the `NEKRS_HOME` variable
as appropriate for your build location. In addition, if you are running any inputs
that use OpenMC, be sure to also set the `OPENMC_CROSS_SECTIONS` variable to the
location where the OpenMC cross sections are located. Remember that you do not need
this cross section data if you are only using the Nek wrapping.

## Eddy (8/30/2021)

The following `~/.bashrc` allows you to compile Cardinal on
[Eddy](https://wiki.inside.anl.gov/ne/The_Eddy_Cluster) at ANL.

!listing! language=bash
module purge
module load moose/.mpich-3.3_gcc-9.2.0
module load miniconda moose-tools

export CC=mpicc
export CXX=mpicxx
export FC=mpif90
export F77=mpif77

# Revise for your Cardinal repository location
export PETSC_DIR=$HOME/cardinal/contrib/moose/petsc
export NEKRS_HOME=$HOME/cardinal/install

# Revise for your cross section data location
export OPENMC_CROSS_SECTIONS=$HOME/cross_sections/endfb71_hdf5/cross_sections.xml

export PETSC_ARCH=arch-moose
export HDF5_ROOT=/opt/moose/seacas
!listing-end!

Below is a sample job script that runs a [!ac](CHT) case for Nek coupled to MOOSE
on one node of the 32 code node partition (`eddy32core`).

!listing! language=bash
#!/bin/bash
#PBS -k o
#PBS -l nodes=1:ppn=32,walltime=24:00:00
#PBS -M email@address.gov
#PBS -m ae
#PBS -N sfr_pin
#PBS -j oe
#PBS -q eddy32core

cd $HOME/cardinal/test/tests/cht/sfr_pincell

module load moose/.mpich-3.3_gcc-9.2.0
module load miniconda moose-tools
export NEKRS_HOME=$HOME/cardinal/install

mpirun -np 32 $HOME/cardinal/cardinal-opt -i nek_master.i  > logfile
!listing-end!

## LCRC at ANL (5/24/2021)

The following `~/.bashrc` allows you to compile Cardinal on
[LCRC](https://www.lcrc.anl.gov/systems/resources/) at ANL.

!listing! language=bash
if [ -f /etc/bashrc ]; then
        . /etc/bashrc
fi

# Revise for your Cardinal repository location
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

# Revise for your cross section data location
export OPENMC_CROSS_SECTIONS=$HOME/cross_sections/endfb71_hdf5/cross_sections.xml

!listing-end!

## Sawtooth at INL (4/29/2021)

The following `~/.bashrc` allows you to compile Cardinal on Sawtooth.

!listing! language=bash
if [ -f /etc/bashrc ]; then
        . /etc/bashrc
fi

# Revise for your Cardinal repository location
export NEKRS_HOME=$HOME/cardinal/install

module purge
module load openmpi/4.0.5_ucx1.9
module load cmake/3.16.2-gcc-9.3.0-tza7
module load hdf5/1.12.0_ucx1.9

export CC=mpicc
export CXX=mpicxx
export FC=mpif90

# Revise for your cross section data location
export OPENMC_CROSS_SECTIONS=$HOME/cross_sections/endfb71_hdf5/cross_sections.xml
!listing-end!

## Summit at ORNL (8/1/2021)

The following `~/.bashrc` allows you to compile Cardinal on Summit.

!listing! language=bash
# Revise for your Cardinal repository location
export NEKRS_HOME=/gpfs/alpine/csc404/proj-shared/novak/cardinal/install

module load gcc cmake cuda hdf5 python/3.7.0-anaconda3-5.3.0

# Revise for your cross section data location
export OPENMC_CROSS_SECTIONS=$HOME/cross_sections/endfb71_hdf5/cross_sections.xml
!listing-end!
