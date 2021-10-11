# HPC Systems

The Cardinal development team regularly builds and runs Cardinal on a number
of [!ac](HPC) systems. To help streamline the compilation
process for other users, this section collects instructions for the modules and any
systems-specific settings for a variety of [!ac](HPC) systems. Because default modules and
settings change on [!ac](HPC) systems with time, the instructions below may become deprecated.
The date at which the settings were last tested is given for each system.
Note that the absence of a particular [!ac](HPC) system from this list does not imply that Cardinal will not
build/run on that system - only that no instructions have yet been documented for it.

Also please note that in addition to these provided module and environment settings,
you must also follow the build instructions on the
[Getting Started](start.md) page. Two environment variables that are important to set
for your particular setup are:

- `NEKRS_HOME`: should point to NekRS install location, or `cardinal/install`; this
   is only needed for inputs that run NekRS
- `OPENMC_CROSS_SECTIONS`: should point to the location of `cross_sections.xml`; this
   is only needed for inputs that run OpenMC

## Eddy

[Eddy](https://wiki.inside.anl.gov/ne/The_Eddy_Cluster) is a cluster at
[!ac](ANL) with eleven 32-core nodes, five 40-core nodes, and six 80-core nodes. Below are
a bash script and sample job scripts to build Cardinal and run the NekRS and OpenMC
wrappings (*last updated 10/11/2021*)

!listing! language=bash caption=`~/.bashrc` to compile Cardinal id=e1
module purge
module load moose/.mpich-3.3_gcc-9.2.0
module load miniconda moose-tools

export CC=mpicc
export CXX=mpicxx
export FC=mpif90
export F77=mpif77

export PETSC_ARCH=arch-moose
export HDF5_ROOT=/opt/moose/seacas

# Revise for your Cardinal repository location
export PETSC_DIR=$HOME/cardinal/contrib/moose/petsc
!listing-end!

!listing! language=bash caption=Sample job script to run OpenMC coupled to MOOSE on one nore of the 32-core partition with 32 OpenMP threads id=e3
#!/bin/bash
#PBS -k o
#PBS -l nodes=1:ppn=32
#PBS -l walltime=5:00
#PBS -M email@address.gov
#PBS -m ae
#PBS -N lattice
#PBS -j oe
#PBS -q eddy32core

module purge
module load moose/.mpich-3.3_gcc-9.2.0
module load miniconda moose-tools

# Revise for your cross section data location
export OPENMC_CROSS_SECTIONS=$HOME/cross_sections/endfb71_hdf5/cross_sections.xml

# Revise for your input file and executable locations
cd $HOME/cardinal/test/tests/neutronics/feedback/lattice
mpirun -np 1 $HOME/cardinal/cardinal-opt -i openmc_master.i --n-threads=32 > logfile
!listing-end!

!listing! language=bash caption=Sample job script to run Nek coupled to MOOSE on one node of the 32-core partition (`eddy32core`) with 32 MPI ranks id=e2
#!/bin/bash
#PBS -k o
#PBS -l nodes=1:ppn=32
#PBS -l walltime=5:00
#PBS -M email@address.gov
#PBS -m ae
#PBS -N sfr_pin
#PBS -j oe
#PBS -q eddy32core

module purge
module load moose/.mpich-3.3_gcc-9.2.0
module load miniconda moose-tools

# Revise for your Cardinal repository location
export NEKRS_HOME=$HOME/cardinal/install

# Revise for your input file and executable locations
cd $HOME/cardinal/test/tests/cht/sfr_pincell
mpirun $HOME/cardinal/cardinal-opt -i nek_master.i  > logfile
!listing-end!

## Bebop

[Bebop](https://www.lcrc.anl.gov/systems/resources/bebop/)
is an [!ac](HPC) system at [!ac](ANL) with 1,024 nodes with an Intel Broadwell
partition with 36 cores/node and a Intel Knights Landing partition with
64 cores/node. Below is a bash script to build Cardinal (*last updated 5/24/2021*).

!listing! language=bash caption=`~/.bashrc` to compile Cradinal id=bb1
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
!listing-end!

## Sawtooth

[Sawtooth](https://nsuf.inl.gov/Page/computing_resources)
 is an [!ac](HPC) system at [!ac](INL) with 99,792 cores. Each compute node contains
dual Xeon Platinum 8268 processors with 24 cores each, giving 48 cores per node. 27 nodes have
four NVIDIA V100 GPUs each. Below are a bash script and sample job scripts to build Cardinal and
run the NekRS and OpenMC wrappings (*last updated 10/11/2021*).

!listing! language=bash caption=`~/.bashrc` to compile Cardinal id=st1
if [ -f /etc/bashrc ]; then
        . /etc/bashrc
fi

if [ -f  ~/.bashrc_local ]; then
       . ~/.bashrc_local
fi

module purge
module load openmpi/4.0.5_ucx1.9
module load cmake/3.16.2-gcc-9.3.0-tza7
module load hdf5/1.12.0_ucx1.9

export CC=mpicc
export CXX=mpicxx
export FC=mpif90
!listing-end!

!listing! language=bash caption=Job script to run OpenMC coupled to MOOSE on one node with the `moose` project code for 2 MPI processes and 24 OpenMP threads id=st2
#!/bin/bash
#PBS -l select=1:ncpus=48:mpiprocs=2:ompthreads=24
#PBS -l walltime=5:00
#PBS -M email@address.gov
#PBS -m ae
#PBS -N lattice
#PBS -j oe
#PBS -P moose

module purge
module load openmpi/4.0.5_ucx1.9
module load hdf5/1.12.0_ucx1.9

# Revise for your cross section data location
export OPENMC_CROSS_SECTIONS=$HOME/projects/cross_sections/endfb71_hdf5/cross_sections.xml

# Review for your input file and executable locations
cd $HOME/projects/cardinal/test/tests/neutronics/feedback/lattice
mpirun $HOME/projects/cardinal/cardinal-opt -i openmc_master.i --n-threads=24 > logfile
!listing-end!

!listing! language=bash caption=Job script to run NekRS coupled to MOOSE on one node with the `moose` project code for 48 MPI processes id=st3
#!/bin/bash
#PBS -l select=1:ncpus=48:mpiprocs=48
#PBS -l walltime=5:00
#PBS -M email@address.gov
#PBS -m ae
#PBS -N pebble
#PBS -j oe
#PBS -P moose

module purge
module load openmpi/4.0.5_ucx1.9
module load cmake/3.16.2-gcc-9.3.0-tza7
module load hdf5/1.12.0_ucx1.9

# Revise for your Cardinal repository location
export NEKRS_HOME=$HOME/projects/cardinal/install

# Revise for your input file and executable locations
cd $HOME/projects/cardinal/test/tests/cht/pebble
mpirun $HOME/projects/cardinal/cardinal-opt -i nek_master.i > logfile
!listing-end!

## Summit

[Summit](https://docs.olcf.ornl.gov/systems/summit_user_guide.html)
is an [!ac](HPC) system at [!ac](ORNL) with approximately
4,600 compute nodes, each of which has two IBM POWER9 processors
and six NVIDIA Tesla V100 GPUs. Below is a bash script to build
Cardinal on Summit (*last updated 8/01/2021*).

!listing! language=bash caption=`~/.bashrc` to compile Cardinal id=su1
module load gcc
module load cmake
module load cuda
module load hdf5
module load python/3.7.0-anaconda3-5.3.0
!listing-end!
